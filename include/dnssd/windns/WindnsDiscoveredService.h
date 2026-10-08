#pragma once

#if !_WIN32 || !USE_WINDNS
#error This file should only be included on Windows builds and have the 'USE_WINDNS' compilation flag enabled.
#endif

#include "Windns.h"
#include "dnssd/ServiceDescription.h"

#include <cstdint>

namespace dnssd
{

/**
 * Tracks all resolved state for a single discovered service instance.
 */
struct WindnsDiscoveredService
{
    /// The service description, filled incrementally as queries complete.
    ServiceDescription description;

    /// Unique id of this entry. Async contexts carry this id so that a late callback for a removed entry is not
    /// applied to a newer entry with the same instance name.
    uint64_t id = 0;

    /// Cancel handle for the in-flight DnsServiceResolve call.
    DNS_SERVICE_CANCEL resolveCancel {};

    /// Cancel handle for the in-flight A record (IPv4) query.
    DNS_QUERY_CANCEL aCancel {};

    /// Cancel handle for the in-flight AAAA record (IPv6) query.
    DNS_QUERY_CANCEL aaaaCancel {};

    /// True while a DnsServiceResolve call is in-flight (cancel handle is live).
    bool resolveInFlight = false;

    /// True once the service has been resolved successfully at least once.
    bool resolved = false;

    /// True while a DnsQueryEx A (IPv4) call is in-flight (cancel handle is live).
    bool aQueryInFlight = false;

    /// True while a DnsQueryEx AAAA (IPv6) call is in-flight (cancel handle is live).
    bool aaaaQueryInFlight = false;
};

} // namespace dnssd
