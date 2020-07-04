//
// Created by Ruurd Adema on 28/06/2020.
//

#include <dnssd/Browser.h>
#include <dnssd/internal/Debug.h>

#include <string>
#include <thread>

void DNSSD_API browseReply2(DNSServiceRef browseServiceRef, DNSServiceFlags flags, uint32_t interfaceIndex,
                           DNSServiceErrorType errorCode, const char* name, const char* type,
                           const char* domain, void* context)
{
    auto* browser = static_cast<dnssd::Browser*>(context);
    browser->browseReply(browseServiceRef, flags, interfaceIndex, errorCode, name, type, domain);
}

dnssd::Browser::Browser(const Listener& listener):
    mListener(listener),
    mThread(std::thread(&Browser::thread, this))
{
}

void dnssd::Browser::browseReply(DNSServiceRef browseServiceRef, DNSServiceFlags inFlags, uint32_t interfaceIndex,
                                 DNSServiceErrorType errorCode, const char* name, const char* type, const char* domain)
{
    DNSSD_LOG_DEBUG("> browseReply enter (context=" << this << ")" << std::endl)

    if (reportIfError(errorCode)) { return; }

    DNSSD_LOG_DEBUG(
        "> browseReply"
        << " name=" << name
        << " type=" << type
        << " domain=" << domain
        << " browseServiceRef=" << browseServiceRef
        << " interfaceIndex=" << interfaceIndex
        << std::endl)

    char fullname[kDNSServiceMaxDomainName] = {};
    if (reportIfError(DNSServiceConstructFullName(fullname, name, type, domain))) { return; }

    if (inFlags & kDNSServiceFlagsAdd)
    {
        // Insert a new service if not already present
        auto service = mServices.find(fullname);
        if (service == mServices.end())
        {
            service = mServices.insert({fullname, Service(fullname, name, type, domain, *this)}).first;
            callListener([&service](const Listener& observer){
                observer.onServiceDiscoveredAsync(service->second.description());
            });
        }

        service->second.resolveOnInterface(interfaceIndex);
    }
    else
    {
        auto foundService = mServices.find(fullname);
        if (foundService == mServices.end())
        {
            if (reportIfError({std::string("service with fullname \"") + fullname + "\" not found"}))
            {
                return;
            }
        }

        if (foundService->second.removeInterface(interfaceIndex) == 0)
        {
            // We just removed the last interface
            callListener([&foundService](const Listener& observer){
                observer.onServiceRemovedAsync(foundService->second.description());
            });

            // Remove the BrowseResult (as there are not interfaces left)
            mServices.erase(foundService);
        }
    }

    DNSSD_LOG_DEBUG("< browseReply exit (" << std::this_thread::get_id() << ")" << std::endl)
}

void dnssd::Browser::callListener(const std::function<void(const Listener&)>& callback) const noexcept
{
    callback(mListener);
}
bool dnssd::Browser::reportIfError(const dnssd::Error& error) const noexcept
{
    if (error)
    {
        callListener([error](const Listener& listener) {
            listener.onBrowserErrorAsync(error);
        });
        return true;
    }
    return false;
}

dnssd::Error dnssd::Browser::browseFor(const std::string& service)
{
    if (mBrowsers.find(service) != mBrowsers.end())
    {
        // Already browsing for this service
        return {"already browsing for service \"" + service + "\""};
    }

    // Initialize with the shared connection to pass it to DNSServiceBrowse
    DNSServiceRef browsingServiceRef = mSharedConnection;

    if (auto error = dnssd::Error(DNSServiceBrowse(
        &browsingServiceRef,
        kDNSServiceFlagsShareConnection,
        kDNSServiceInterfaceIndexAny,
        service.c_str(),
        nullptr,
        ::browseReply2,
        this)))
    {
        return error;
    }

    mBrowsers.insert({service, ScopedDNSServiceRef(browsingServiceRef)});
    // From here the serviceRef is under RAII inside the ScopedDNSServiceRef class

    return dnssd::Error();
}

void dnssd::Browser::thread()
{
    DNSSD_LOG_DEBUG("> Start browse thread" << std::endl)

    struct timeval tv = {};
    tv.tv_sec  = 0;
    tv.tv_usec = 500000;

    int fd = -1;
    int nfds = -1;

    fd = DNSServiceRefSockFD(mSharedConnection);

    if (fd < 0)
    {
        if (reportIfError({"Invalid file descriptor"}))
        {
            DNSSD_LOG_DEBUG("< Invalid file descriptor" << std::endl)
            return;
        }
    }

    while (mKeepGoing.load())
    {
        fd_set readfds;
        FD_ZERO(&readfds);
        FD_SET(fd, &readfds);
        nfds = fd + 1;

        int result = select(nfds, &readfds, (fd_set*)nullptr, (fd_set*)nullptr, &tv);

        if (result < 0) // Error
        {
            if (reportIfError({"Select error: " + std::to_string(result)}))
            {
                DNSSD_LOG_DEBUG("! Error (code=" << result << ")" << std::endl)
                break;
            }
        }
        else if (result == 0) // Timeout
        {
        }
        else
        {
            if (FD_ISSET(fd, &readfds))
            {
                DNSSD_LOG_DEBUG("> Main loop (FD_ISSET == true)" << std::endl)
                (void) reportIfError(DNSServiceProcessResult(mSharedConnection));
            }
            else
            {
                DNSSD_LOG_DEBUG("> Main loop (FD_ISSET == false)" << std::endl)
            }
        }
    }

    DNSSD_LOG_DEBUG("< Stop browse thread" << std::endl)
}
dnssd::Browser::~Browser()
{
    mKeepGoing = false;
    if (mThread.joinable()) { mThread.join(); }
}
