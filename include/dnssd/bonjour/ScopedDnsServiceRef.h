#pragma once

#include "dnssd/common/Result.h"

namespace dnssd
{

/**
 * RAII wrapper around DNSServiceRef.
 */
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

    /**
     * Assigns an existing DNSServiceRef to this instance. An existing DNSServiceRef will be deallocated, and this
     * object will take ownership over the given DNSServiceRef.
     * @param serviceRef The DNSServiceRef to assign to this instance.
     * @return A reference to this instance.
     */
    ScopedDnsServiceRef& operator= (DNSServiceRef serviceRef);

    /**
     * @return Returns the contained DNSServiceRef.
     */
    DNSServiceRef serviceRef() const noexcept { return mServiceRef; }

private:
    DNSServiceRef mServiceRef = nullptr;
};

} // namespace dnssd