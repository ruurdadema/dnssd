#pragma once

#include "dnssd/ServiceDescription.h"
#include "dnssd/bonjour/ScopedDnsServiceRef.h"

namespace dnssd
{

class BonjourBrowser;

/**
 * Represents a Bonjour service and holds state and methods for discovering and resolving services on the network.
 */
class Service
{
public:
    /**
     * Constructs a service.
     * @param fullname Full domain name of the service.
     * @param name Name of the service.
     * @param type Type of the service (ie. _http._tcp)
     * @param domain The domain of the service (ie. local.).
     * @param owner A reference to the owning BonjourBrowser.
     */
    Service (const char* fullname, const char* name, const char* type, const char* domain, BonjourBrowser& owner);

    /**
     * Called when a service was resolved.
     * @param index The index of the interface the service was resolved on.
     */
    void resolveOnInterface (uint32_t index);

    /**
     * Called when an interface was removed for this service.
     * @param index The index of the removed interface.
     * @return The amount of interfaces after the removal.
     */
    size_t removeInterface (uint32_t index);

    /**
     * @return Returns the ServiceDescription.
     */
    const ServiceDescription& description() const noexcept { return mDescription; }

    /**
     * Called by dns_sd callbacks when a service was resolved.
     * @param serviceRef The DNSServiceRef.
     * @param flags Possible values: kDNSServiceFlagsMoreComing.
     * @param interfaceIndex The interface on which the service was resolved.
     * @param errorCode Will be kDNSServiceErr_NoError (0) on success, otherwise will indicate the failure that
     * occurred. Other parameters are undefined if the errorCode is nonzero.
     * @param fullname The full service domain name, in the form <servicename>.<protocol>.<domain>. (This name is
     *                  escaped following standard DNS rules, making it suitable for passing to standard system DNS APIs
     *                  such as res_query(), or to the special-purpose functions included in this API that take fullname
     *                  parameters. See "Notes on DNS Name Escaping" earlier in this file for more details.)
     * @param hosttarget The target hostname of the machine providing the service. This name can be passed to functions
     *                   like gethostbyname() to identify the host's IP address.
     * @param port The port, in network byte order, on which connections are accepted for this service.
     * @param txtLen The length of the txt record, in bytes.
     * @param txtRecord The service's primary txt record, in standard txt record format.
     */
    void resolveCallBack (
        DNSServiceRef serviceRef,
        DNSServiceFlags flags,
        uint32_t interfaceIndex,
        DNSServiceErrorType errorCode,
        const char* fullname,
        const char* hosttarget,
        uint16_t port, // In network byte order.
        uint16_t txtLen,
        const unsigned char* txtRecord);

    /**
     * Called by dns_sd callbacks when an address was resolved.
     * @param sdRef The DNSServiceRef
     * @param flags Possible values are kDNSServiceFlagsMoreComing and kDNSServiceFlagsAdd.
     * @param interfaceIndex The interface to which the answers pertain.
     * @param errorCode Will be kDNSServiceErr_NoError on success, otherwise will indicate the failure that occurred.
     *                  Other parameters are undefined if errorCode is nonzero.
     * @param hostname The fully qualified domain name of the host to be queried for.
     * @param address IPv4 or IPv6 address.
     * @param ttl Time to live.
     */
    void getAddrInfoCallBack (
        DNSServiceRef sdRef,
        DNSServiceFlags flags,
        uint32_t interfaceIndex,
        DNSServiceErrorType errorCode,
        const char* hostname,
        const struct sockaddr* address,
        uint32_t ttl);

private:
    BonjourBrowser& mOwner;
    std::map<uint32_t, ScopedDnsServiceRef> mResolvers;
    std::map<uint32_t, ScopedDnsServiceRef> mGetAddrs;
    ServiceDescription mDescription;
};

} // namespace dnssd
