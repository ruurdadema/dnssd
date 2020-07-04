//
// Created by Ruurd Adema on 04/07/2020.
//

#pragma once

#include "../internal/Error.h"
#include "../Advertiser.h"

#include <map>

namespace dnssd {

    class BonjourAdvertiserImpl : public Advertiser::Impl
    {
    public:
        explicit BonjourAdvertiserImpl(const Advertiser::Listener& listener);

        Error registerService(const std::string& serviceName, uint16_t port) noexcept override;
        Error registerService(const std::string& serviceName, uint16_t port, const TXTRecord& txtRecord) noexcept override;
        Error registerService(const std::string& serviceName,
                              uint16_t port,
                              const std::map<std::string, std::string>& keysValues) noexcept override;

        void unregisterService() noexcept override;
        void callObserver(std::function<void(const Advertiser::Listener&)>) noexcept;

    private:
        DNSServiceRef mServiceRef = nullptr; // TODO: Make this a ScopedDNSServiceRef
        const Advertiser::Listener& mListener;
    };

} // namespace dnssd
