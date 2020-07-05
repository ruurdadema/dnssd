//
// Created by Ruurd Adema on 28/06/2020.
//

#include <dnssd/bonjour/ScopedDNSServiceRef.h>
#include <utility>

dnssd::ScopedDNSServiceRef::~ScopedDNSServiceRef()
{
    if (mServiceRef != nullptr)
    {
        DNSServiceRefDeallocate(mServiceRef);
    }
}

dnssd::ScopedDNSServiceRef::ScopedDNSServiceRef(ScopedDNSServiceRef&& other) noexcept
{
    *this = std::move(other);
}

dnssd::ScopedDNSServiceRef::ScopedDNSServiceRef(const DNSServiceRef& serviceRef) noexcept : mServiceRef(serviceRef)
{
}

dnssd::ScopedDNSServiceRef& dnssd::ScopedDNSServiceRef::operator=(dnssd::ScopedDNSServiceRef&& other)
{
    if (mServiceRef != nullptr)
    {
        DNSServiceRefDeallocate(mServiceRef);
    }
    mServiceRef = other.mServiceRef;
    other.mServiceRef = nullptr;
    return *this;
}
dnssd::ScopedDNSServiceRef& dnssd::ScopedDNSServiceRef::operator=(DNSServiceRef& serviceRef)
{
    mServiceRef = serviceRef;
    return *this;
}
