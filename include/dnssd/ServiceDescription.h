/*
  ==============================================================================

    BrowseResult.h
    Created: 22 Jun 2020 2:09:17pm
    Author:  Ruurd Adema

  ==============================================================================
*/

#pragma once

#include <string>
#include <map>
#include <set>

namespace dnssd {

struct ServiceDescription
{
    std::string fullname;
    std::string name;
    std::string type;
    std::string domain;
    std::string hostTarget;
    uint16_t port;
    std::map<std::string, std::string> txtRecord;
    std::map<uint32_t, std::set<std::string>> interfaces; // interfaceIndex, addresses

    [[nodiscard]] std::string description() const noexcept;
};

} // namespace dnssd
