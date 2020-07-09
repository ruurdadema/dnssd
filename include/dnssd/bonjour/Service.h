//
// Created by Ruurd Adema on 28/06/2020.
//

#pragma once

#include "dnssd/ServiceDescription.h"
#include "dnssd/bonjour/ScopedDnsServiceRef.h"

namespace dnssd {

    class BonjourBrowser;

    class Service
    {
    public:
        Service(const char* fullname,
                const char* name,
                const char* type,
                const char* domain,
                const BonjourBrowser& owner);

        void resolveOnInterface(uint32_t index);
        size_t removeInterface(uint32_t index);
        [[nodiscard]] const ServiceDescription& description() const noexcept { return mDescription; }

        void resolveCallBack(DNSServiceRef serviceRef, DNSServiceFlags flags, uint32_t interfaceIndex,
            DNSServiceErrorType errorCode, const char* fullname, const char* hosttarget,
            uint16_t port /*In network byte order*/, uint16_t txtLen, const unsigned char* txtRecord);

        void getAddrInfoCallBack(DNSServiceRef sdRef, DNSServiceFlags flags, uint32_t interfaceIndex,
            DNSServiceErrorType errorCode, const char* hostname, const struct sockaddr* address, uint32_t ttl);

    private:
        const BonjourBrowser& mOwner;
        std::map<uint32_t, ScopedDnsServiceRef> mResolvers;
        std::map<uint32_t, ScopedDnsServiceRef> mGetAddrs;
        ServiceDescription mDescription;
    };

} // namespace dnssd
