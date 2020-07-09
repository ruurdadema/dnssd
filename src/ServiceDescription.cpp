#include <dnssd/ServiceDescription.h>
#include <sstream>

using namespace dnssd;

std::string ServiceDescription::description() const noexcept
{
    std::string txtRecordDescription;

    for (auto& kv : txtRecord)
    {
        txtRecordDescription += kv.first;
        txtRecordDescription += "=";
        txtRecordDescription += kv.second;
        txtRecordDescription += ", ";
    }

    std::string addressesDescription;

    for (auto& interface : interfaces)
    {
        addressesDescription += "interface ";
        addressesDescription += std::to_string(interface.first);
        addressesDescription += ": ";

        for (auto& addr : interface.second)
        {
            addressesDescription += addr;
            addressesDescription += ", ";
        }
    }

    std::stringstream output;

    output
    << "fullname: " << fullname
    << ", name: " << name
    << ", type: " << type
    << ", domain: " << domain
    << ", hostTarget: " << hostTarget
    << ", port: " << port
    << ", txtRecord: " << txtRecordDescription
    << "addresses: " << addressesDescription;

    return output.str();
}
