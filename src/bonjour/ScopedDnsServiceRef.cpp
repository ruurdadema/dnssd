//
// Created by Ruurd Adema on 28/06/2020.
//

#include <dnssd/bonjour/ScopedDnsServiceRef.h>
#include <utility>

dnssd::ScopedDnsServiceRef::~ScopedDnsServiceRef()
{
    if (mServiceRef != nullptr)
    {
        DNSServiceRefDeallocate(mServiceRef);
    }
}

dnssd::ScopedDnsServiceRef::ScopedDnsServiceRef(ScopedDnsServiceRef&& other) noexcept
{
    *this = std::move(other);
}

dnssd::ScopedDnsServiceRef::ScopedDnsServiceRef(const DNSServiceRef& serviceRef) noexcept : mServiceRef(serviceRef)
{
}

dnssd::ScopedDnsServiceRef& dnssd::ScopedDnsServiceRef::operator=(dnssd::ScopedDnsServiceRef&& other)
{
    if (mServiceRef != nullptr)
    {
        DNSServiceRefDeallocate(mServiceRef);
    }
    mServiceRef = other.mServiceRef;
    other.mServiceRef = nullptr;
    return *this;
}
dnssd::ScopedDnsServiceRef& dnssd::ScopedDnsServiceRef::operator=(DNSServiceRef& serviceRef)
{
    mServiceRef = serviceRef;
    return *this;
}
