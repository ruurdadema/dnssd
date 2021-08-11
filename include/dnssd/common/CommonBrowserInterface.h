#pragma once

#include "../ServiceDescription.h"
#include "Error.h"

#include <functional>

namespace dnssd
{

class CommonBrowserInterface
{
public:
    CommonBrowserInterface() = default;
    virtual ~CommonBrowserInterface() = default;

    virtual Error browseFor (const std::string& service) = 0;

    virtual void onServiceDiscoveredAsync (const ServiceDescription& serviceDescription) {}
    virtual void onServiceRemovedAsync (const ServiceDescription& serviceDescription) {}
    virtual void onServiceResolvedAsync (const ServiceDescription& serviceDescription, uint32_t interfaceIndex) {}
    virtual void onAddressAddedAsync (
        const ServiceDescription& serviceDescription,
        const std::string& address,
        uint32_t interfaceIndex) {};
    virtual void onAddressRemovedAsync (
        const ServiceDescription& serviceDescription,
        const std::string& address,
        uint32_t interfaceIndex) {};
    virtual void onBrowserErrorAsync (const Error& error) {}
};

} // namespace dnssd
