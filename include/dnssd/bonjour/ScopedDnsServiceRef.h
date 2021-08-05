#pragma once

#include "dnssd/common/Error.h"

namespace dnssd
{

class ScopedDnsServiceRef
{
public:
    ScopedDnsServiceRef() = default;
    ~ScopedDnsServiceRef();
    ScopedDnsServiceRef (const ScopedDnsServiceRef&) = delete;
    ScopedDnsServiceRef (ScopedDnsServiceRef&& other) noexcept;
    explicit ScopedDnsServiceRef (const DNSServiceRef& serviceRef) noexcept;

    ScopedDnsServiceRef& operator= (const ScopedDnsServiceRef& other) = delete;
    ScopedDnsServiceRef& operator= (ScopedDnsServiceRef&& other) noexcept;
    ScopedDnsServiceRef& operator= (DNSServiceRef serviceRef);

    DNSServiceRef serviceRef() const noexcept { return mServiceRef; }

private:
    DNSServiceRef mServiceRef = nullptr;
};

} // namespace dnssd