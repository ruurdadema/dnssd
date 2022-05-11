#pragma once

#include "common/TxtRecord.h"

#include <map>
#include <set>
#include <string>

namespace dnssd
{

/**
 * A struct containing data which represents a service on the network.
 */
struct ServiceDescription
{
    /// The full service domain name.
    std::string fullname;

    /// The name of the service.
    std::string name;

    /// The type of the service (ie. _http._tcp.).
    std::string type;

    /// The domain of the service (local.).
    std::string domain;

    /// The host target of the service (name.local.).
    std::string hostTarget;

    /// The port of the service (in native endian).
    uint16_t port;

    /// The TXT record of the service, represented as a map of keys and values.
    TxtRecord txtRecord;

    /// The resolved addresses of this service.
    std::map<uint32_t, std::set<std::string>> interfaces; // interfaceIndex, addresses

    /// Returns a description of this struct, which might be handy for debugging or logging purposes.
    std::string description() const noexcept;
};

} // namespace dnssd
