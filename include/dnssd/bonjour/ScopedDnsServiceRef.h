//
// Created by Ruurd Adema on 28/06/2020.
//

#pragma once

#include "dnssd/common/Error.h"

namespace dnssd {

    class ScopedDnsServiceRef
    {
    public:
        ScopedDnsServiceRef() = default;
        ~ScopedDnsServiceRef();
        ScopedDnsServiceRef(const ScopedDnsServiceRef&) = delete;
        ScopedDnsServiceRef(ScopedDnsServiceRef&& other) noexcept;
        ScopedDnsServiceRef(const DNSServiceRef& serviceRef) noexcept;

        ScopedDnsServiceRef& operator=(const ScopedDnsServiceRef& other) = delete;
        ScopedDnsServiceRef& operator=(ScopedDnsServiceRef&& other);
        ScopedDnsServiceRef& operator=(DNSServiceRef& serviceRef);

        operator DNSServiceRef() const noexcept { return mServiceRef; }

    private:
        DNSServiceRef mServiceRef = nullptr;
    };

} // namespace dnssd