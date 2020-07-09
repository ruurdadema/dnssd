#include <dnssd/bonjour/Service.h>
#include <dnssd/common/Log.h>
#include <dnssd/bonjour/BonjourBrowser.h>

#include <thread>
#include <dnssd/Browser.h>
#include <dnssd/bonjour/BonjourTxtRecord.h>
#include <map>

void DNSSD_API resolveCallBack(DNSServiceRef sdRef, DNSServiceFlags flags, uint32_t interfaceIndex,
                               DNSServiceErrorType errorCode, const char* fullname, const char* hosttarget, uint16_t port, // In network byte order
                               uint16_t txtLen, const unsigned char* txtRecord, void* context)
{
    auto* service = static_cast<dnssd::Service*>(context);
    service->resolveCallBack(sdRef, flags, interfaceIndex, errorCode, fullname, hosttarget, port, txtLen, txtRecord);
}

void DNSSD_API getAddrInfoCallBack(DNSServiceRef sdRef, DNSServiceFlags flags, uint32_t interfaceIndex,
                                   DNSServiceErrorType errorCode, const char* fullname,
                                   const struct sockaddr* address, uint32_t ttl, void* context)
{
    auto* service = static_cast<dnssd::Service*>(context);
    service->getAddrInfoCallBack(sdRef, flags, interfaceIndex, errorCode, fullname, address, ttl);
}

dnssd::Service::Service(const char* fullname,
                        const char* name,
                        const char* type,
                        const char* domain,
                        const BonjourBrowser& owner) : mOwner(owner)
{
    mDescription.fullname = fullname;
    mDescription.name     = name;
    mDescription.type     = type;
    mDescription.domain   = domain;
}

void dnssd::Service::resolveOnInterface(uint32_t index)
{
    if (mResolvers.find(index) != mResolvers.end())
    {
        // Already resolving on this interface
        return;
    }

    mDescription.interfaces.insert({index, {}});

    DNSServiceRef resolveServiceRef = mOwner.sharedConnection().serviceRef();

    if (mOwner.reportIfError(Error(DNSServiceResolve(&resolveServiceRef, kDNSServiceFlagsShareConnection,
        index, mDescription.name.c_str(),mDescription.type.c_str(), mDescription.domain.c_str(),
        ::resolveCallBack, this)))) { return; }

    mResolvers.insert({index, ScopedDnsServiceRef(resolveServiceRef)});
}

void dnssd::Service::resolveCallBack(DNSServiceRef serviceRef, DNSServiceFlags flags, uint32_t interfaceIndex,
    DNSServiceErrorType errorCode, const char* fullname, const char* hosttarget, uint16_t port, uint16_t txtLen,
    const unsigned char* txtRecord)
{
    DNSSD_LOG_DEBUG("> resolveCallBack enter (" << std::this_thread::get_id() << ") context=" << this << std::endl)
    DNSSD_LOG_DEBUG("- fullname=" << fullname << std::endl);

    if (mOwner.reportIfError(Error(errorCode))) { return; }

    mDescription.hostTarget = hosttarget;
    mDescription.port = port;
    mDescription.txtRecord = BonjourTxtRecord::getTxtRecordFromRawBytes(txtRecord, txtLen);

    DNSSD_LOG_DEBUG("- resolveCallBack: " << mDescription.description() << std::endl)

    mOwner.callListener([this, interfaceIndex](const Browser::Listener& listener) {
        listener.onServiceResolvedAsync(mDescription, interfaceIndex);
    });

    DNSServiceRef getAddrInfoServiceRef = mOwner.sharedConnection().serviceRef();

    if (mOwner.reportIfError(Error(DNSServiceGetAddrInfo(&getAddrInfoServiceRef,
        kDNSServiceFlagsShareConnection | kDNSServiceFlagsTimeout, interfaceIndex,
        kDNSServiceProtocol_IPv4 | kDNSServiceProtocol_IPv6, hosttarget, ::getAddrInfoCallBack,this))))
    {
        return;
    }

    mGetAddrs.insert({interfaceIndex, ScopedDnsServiceRef(getAddrInfoServiceRef)});

    DNSSD_LOG_DEBUG("< resolveCallBack exit (" << std::this_thread::get_id() << ")" << std::endl)
}

void dnssd::Service::getAddrInfoCallBack(DNSServiceRef sdRef, DNSServiceFlags flags,
    uint32_t interfaceIndex, DNSServiceErrorType errorCode, const char* hostname, const struct sockaddr* address,
    uint32_t ttl)
{
    DNSSD_LOG_DEBUG("> getAddrInfoCallBack enter (" << std::this_thread::get_id() << ") context=" << this << std::endl)
    DNSSD_LOG_DEBUG("- getAddrInfoCallBack hostname: " << hostname << std::endl)

    if (errorCode == kDNSServiceErr_Timeout)
    {
        mGetAddrs.erase(interfaceIndex);
        return;
    }

    if (mOwner.reportIfError(Error(errorCode))) { return; }

    char ip_addr[INET6_ADDRSTRLEN] = {};

    const void* ip_addr_data = nullptr;

    if (address->sa_family == AF_INET)
    {
        ip_addr_data = &reinterpret_cast<const sockaddr_in*>(address)->sin_addr;
    }
    else if (address->sa_family == AF_INET6)
    {
        ip_addr_data = &reinterpret_cast<const sockaddr_in6*>(address)->sin6_addr;
    }
    else
    {
        return; // Don't know how to handle this case
    }

    // Winsock version requires the const cast because Microsoft.
    inet_ntop(address->sa_family, const_cast<void*>(ip_addr_data), ip_addr, INET6_ADDRSTRLEN);

    auto foundInterface = mDescription.interfaces.find(interfaceIndex);
    if (foundInterface != mDescription.interfaces.end())
    {
        auto result = foundInterface->second.insert(ip_addr);
        mOwner.callListener([this, interfaceIndex, &result](const Browser::Listener& observer){
            observer.onAddressAddedAsync(mDescription, *result.first, interfaceIndex);
        });
    }
    else
    {
        (void) mOwner.reportIfError(Error(std::string("Interface with id \"")
            + std::to_string(interfaceIndex)
            + "\" not found"));
    }

    DNSSD_LOG_DEBUG("- Address: " << ip_addr << std::endl)
    DNSSD_LOG_DEBUG("< getAddrInfoCallBack exit (" << std::this_thread::get_id() << ")" << std::endl)
}

size_t dnssd::Service::removeInterface(uint32_t index)
{
    auto foundInterface = mDescription.interfaces.find(index);
    if (foundInterface == mDescription.interfaces.end())
    {
        (void) mOwner.reportIfError(Error(std::string("interface with index \"")
            + std::to_string(index)
            + "\" not found"));
        return mDescription.interfaces.empty();
    }

    if (mDescription.interfaces.size() > 1)
    {
        for (auto& addr: foundInterface->second)
        {
            mOwner.callListener([this, &addr, index](const Browser::Listener& listener) {
                listener.onAddressRemovedAsync(mDescription, addr, index);
            });
        }
    }

    mDescription.interfaces.erase(foundInterface);
    mResolvers.erase(index);
    mGetAddrs.erase(index);

    return mDescription.interfaces.size();
}
