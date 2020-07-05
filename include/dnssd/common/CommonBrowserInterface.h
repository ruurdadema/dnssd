//
// Created by Ruurd Adema on 05/07/2020.
//

#pragma once

#include "Error.h"
#include "../ServiceDescription.h"

namespace dnssd {

    class CommonBrowserInterface
    {
    public:
        class Listener
        {
        public:
            virtual ~Listener() = default;

            virtual void onServiceDiscoveredAsync(const ServiceDescription& service) const noexcept = 0;
            virtual void onServiceRemovedAsync(const ServiceDescription& service) const noexcept = 0;
            virtual void onServiceResolvedAsync(const ServiceDescription& service, uint32_t interfaceIndex) const noexcept = 0;
            virtual void onAddressAddedAsync(const ServiceDescription& service, const std::string& address, uint32_t interfaceIndex) const noexcept = 0;
            virtual void onAddressRemovedAsync(const ServiceDescription& service, const std::string& address, uint32_t interfaceIndex) const noexcept = 0;
            virtual void onBrowserErrorAsync(Error error) const noexcept = 0;
        };

        virtual ~CommonBrowserInterface() = default;
        virtual Error browseFor(const std::string& service) = 0;
    };

} // namespace dnssd
