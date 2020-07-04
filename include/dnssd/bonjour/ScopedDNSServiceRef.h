//
// Created by Ruurd Adema on 28/06/2020.
//

#pragma once

#include "dnssd/internal/Error.h"

namespace dnssd {

    class ScopedDNSServiceRef
    {
    public:
        ScopedDNSServiceRef() = default;
        ~ScopedDNSServiceRef();
        ScopedDNSServiceRef(const ScopedDNSServiceRef&) = delete;
        ScopedDNSServiceRef(ScopedDNSServiceRef&& other) noexcept;
        ScopedDNSServiceRef(const DNSServiceRef& serviceRef) noexcept;

        ScopedDNSServiceRef& operator=(const ScopedDNSServiceRef& other) = delete;
        ScopedDNSServiceRef& operator=(ScopedDNSServiceRef&& other);
        ScopedDNSServiceRef& operator=(DNSServiceRef& serviceRef);

        operator DNSServiceRef() const noexcept { return mServiceRef; }

    private:
        DNSServiceRef mServiceRef = nullptr;
    };

} // namespace dnssd