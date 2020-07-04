//
// Created by Ruurd Adema on 28/06/2020.
//

#pragma once

#include "ServiceDescription.h"
#include "dnssd/internal/Error.h"

#include <string>
#include <thread>
#include <atomic>

namespace dnssd {
    class Browser
    {
    public:
        class Impl
        {
        public:
            virtual ~Impl() = default;
            virtual Error browseFor(const std::string& service) = 0;
        };

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

        explicit Browser(const Listener& listener);

        Error browseFor(const std::string& service);

    private:
        std::unique_ptr<Impl> impl;
    };
}
