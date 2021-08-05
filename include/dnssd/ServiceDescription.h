#pragma once

#include "common/TxtRecord.h"

#include <map>
#include <set>
#include <string>

namespace dnssd
{

struct ServiceDescription
{
    std::string fullname;
    std::string name;
    std::string type;
    std::string domain;
    std::string hostTarget;
    uint16_t port;
    TxtRecord txtRecord;
    std::map<uint32_t, std::set<std::string>> interfaces; // interfaceIndex, addresses

    std::string description() const noexcept;
};

} // namespace dnssd
