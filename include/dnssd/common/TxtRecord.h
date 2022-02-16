#pragma once

#include <map>
#include <string>

namespace dnssd
{

/// Simple typedef for representing a TXT record.
using TxtRecord = std::map<std::string, std::string>;

} // namespace dnssd
