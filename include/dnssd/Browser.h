//
// Created by Ruurd Adema on 28/06/2020.
//

#pragma once

#include "dnssd/bonjour/SharedConnection.h"
#include "ServiceDescription.h"
#include "dnssd/internal/Error.h"
#include "dnssd/internal/Service.h"

#include <string>
#include <thread>
#include <atomic>

namespace dnssd {
    class Browser
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

        Browser(const Listener& listener);
        ~Browser();

        Error browseFor(const std::string& service);

        [[nodiscard]] bool reportIfError(const Error& error) const noexcept;

        void callListener(const std::function<void(const Listener&)>&) const noexcept;

        void browseReply(DNSServiceRef browseServiceRef, DNSServiceFlags inFlags, uint32_t interfaceIndex,
                         DNSServiceErrorType errorCode, const char* name, const char* type,
                         const char* domain);

        [[nodiscard]] const SharedConnection& sharedConnection() const noexcept { return mSharedConnection; }

    private:
        SharedConnection mSharedConnection;
        const Listener& mListener;
        std::map<std::string, ScopedDNSServiceRef> mBrowsers;
        std::map<std::string, Service> mServices;
        std::atomic_bool mKeepGoing = ATOMIC_VAR_INIT(true);
        std::thread mThread;

        void thread();
    };
}
