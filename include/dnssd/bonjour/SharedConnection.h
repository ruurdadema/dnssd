#pragma once

#include "ScopedDnsServiceRef.h"

namespace dnssd
{

/**
 * Represents a shared connection to the mdns responder.
 */
class SharedConnection
{
public:
    /**
     * Constructor which will create a connection and store the DNSServiceRef under RAII fashion.
     */
    SharedConnection();

    /**
     * @return Returns the DNSServiceRef held by this instance. The DNSServiceRef will still be owned by this class.
     */
    DNSServiceRef serviceRef() const noexcept { return mServiceRef.serviceRef(); }

private:
    ScopedDnsServiceRef mServiceRef;
};

} // namespace dnssd
