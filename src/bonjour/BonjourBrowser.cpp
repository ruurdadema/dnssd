#include <dnssd/bonjour/BonjourBrowser.h>
#include <dnssd/bonjour/Service.h>
#include <mutex>

void DNSSD_API browseReply2 (
    DNSServiceRef browseServiceRef,
    DNSServiceFlags flags,
    uint32_t interfaceIndex,
    DNSServiceErrorType errorCode,
    const char* name,
    const char* type,
    const char* domain,
    void* context)
{
    auto* browser = static_cast<dnssd::BonjourBrowser*> (context);
    browser->browseReply (browseServiceRef, flags, interfaceIndex, errorCode, name, type, domain);
}

dnssd::BonjourBrowser::BonjourBrowser() : mThread (std::thread (&BonjourBrowser::thread, this)) {}

void dnssd::BonjourBrowser::browseReply (
    DNSServiceRef browseServiceRef,
    DNSServiceFlags inFlags,
    uint32_t interfaceIndex,
    DNSServiceErrorType errorCode,
    const char* name,
    const char* type,
    const char* domain)
{
    DNSSD_LOG_DEBUG ("> browseReply enter (context=" << this << ")" << std::endl)

    if (reportIfError (Result (errorCode)))
        return;

    DNSSD_LOG_DEBUG (
        "> browseReply"
        << " name=" << name << " type=" << type << " domain=" << domain << " browseServiceRef=" << browseServiceRef
        << " interfaceIndex=" << interfaceIndex << std::endl)

    char fullname[kDNSServiceMaxDomainName] = {};
    if (reportIfError (Result (DNSServiceConstructFullName (fullname, name, type, domain))))
        return;

    if (inFlags & kDNSServiceFlagsAdd)
    {
        // Insert a new service if not already present
        auto service = mServices.find (fullname);
        if (service == mServices.end())
        {
            service = mServices.insert ({ fullname, Service (fullname, name, type, domain, *this) }).first;
            onServiceDiscoveredAsync (service->second.description());
        }

        service->second.resolveOnInterface (interfaceIndex);
    }
    else
    {
        auto foundService = mServices.find (fullname);
        if (foundService == mServices.end())
            if (reportIfError (Result (std::string ("service with fullname \"") + fullname + "\" not found")))
                return;

        if (foundService->second.removeInterface (interfaceIndex) == 0)
        {
            // We just removed the last interface
            onServiceRemovedAsync (foundService->second.description());

            // Remove the BrowseResult (as there are not interfaces left)
            mServices.erase (foundService);
        }
    }

    DNSSD_LOG_DEBUG ("< browseReply exit (" << std::this_thread::get_id() << ")" << std::endl)
}

bool dnssd::BonjourBrowser::reportIfError (const dnssd::Result& error) noexcept
{
    if (error)
    {
        onBrowserErrorAsync (error);
        return true;
    }
    return false;
}

dnssd::Result dnssd::BonjourBrowser::browseFor (const std::string& service)
{
    std::lock_guard<std::recursive_mutex> lg (mLock);

    // Initialize with the shared connection to pass it to DNSServiceBrowse
    DNSServiceRef browsingServiceRef = mSharedConnection.serviceRef();

    if (browsingServiceRef == nullptr)
        return Result (kDNSServiceErr_ServiceNotRunning);

    if (mBrowsers.find (service) != mBrowsers.end())
    {
        // Already browsing for this service
        return Result ("already browsing for service \"" + service + "\"");
    }

    if (auto error = dnssd::Result (DNSServiceBrowse (
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

    mBrowsers.insert ({ service, ScopedDnsServiceRef (browsingServiceRef) });
    // From here the serviceRef is under RAII inside the ScopedDnsServiceRef class

    return dnssd::Result();
}

void dnssd::BonjourBrowser::thread()
{
    DNSSD_LOG_DEBUG ("> Start browse thread" << std::endl)

    struct timeval tv = {};
    tv.tv_sec = 0;
    tv.tv_usec = 500000;

    int fd = -1;
    int nfds = -1;

    fd = DNSServiceRefSockFD (mSharedConnection.serviceRef());

    if (fd < 0)
    {
        if (reportIfError (Result ("Invalid file descriptor")))
        {
            DNSSD_LOG_DEBUG ("< Invalid file descriptor" << std::endl)
            return;
        }
    }

    while (mKeepGoing.load())
    {
        fd_set readfds;
        FD_ZERO (&readfds);
        FD_SET (fd, &readfds);
        nfds = fd + 1;

        int result = select (nfds, &readfds, (fd_set*)nullptr, (fd_set*)nullptr, &tv);

        {
            if (result < 0) // Result
            {
                if (reportIfError (Result ("Select error: " + std::to_string (result))))
                {
                    DNSSD_LOG_DEBUG ("! Result (code=" << result << ")" << std::endl)
                    break;
                }
            }
            else if (result == 0) // Timeout
            {
            }
            else
            {
                if (FD_ISSET (fd, &readfds))
                {
                    if (mKeepGoing.load() == false)
                        return;

                    // Locking here will make sure that all callbacks are synchronised because they are called in
                    // response to DNSServiceProcessResult.
                    std::lock_guard<std::recursive_mutex> lg (mLock);

                    DNSSD_LOG_DEBUG ("> Main loop (FD_ISSET == true)" << std::endl)
                    (void)reportIfError (Result (DNSServiceProcessResult (mSharedConnection.serviceRef())));
                }
                else
                {
                    DNSSD_LOG_DEBUG ("> Main loop (FD_ISSET == false)" << std::endl)
                }
            }
        }
    }

    DNSSD_LOG_DEBUG ("< Stop browse thread" << std::endl)
}

dnssd::BonjourBrowser::~BonjourBrowser()
{
    mKeepGoing = false;

    std::lock_guard<std::recursive_mutex> lg (mLock);
    if (mThread.joinable())
        mThread.join();
}
