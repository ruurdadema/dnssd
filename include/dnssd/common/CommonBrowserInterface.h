#pragma once

#include "Error.h"
#include "../ServiceDescription.h"

namespace dnssd {

    class CommonBrowserInterface
    {
    public:
        CommonBrowserInterface() = default;
        virtual ~CommonBrowserInterface() = default;

        std::function<void(const ServiceDescription&)> onServiceDiscoveredAsync;
        std::function<void(const ServiceDescription&)> onServiceRemovedAsync;
        std::function<void(const ServiceDescription&, uint32_t)> onServiceResolvedAsync;
        std::function<void(const ServiceDescription&, const std::string&, uint32_t)> onAddressAddedAsync;
        std::function<void(const ServiceDescription&, const std::string&, uint32_t)> onAddressRemovedAsync;
        std::function<void(const Error& error)> onBrowserErrorAsync;

        virtual Error browseFor(const std::string& service) = 0;
    };

} // namespace dnssd
