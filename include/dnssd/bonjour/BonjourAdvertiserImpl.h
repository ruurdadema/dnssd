//
// Created by Ruurd Adema on 04/07/2020.
//

#pragma once

#include "../internal/Error.h"
#include "../Advertiser.h"

#include <map>

namespace dnssd
{
    class BonjourAdvertiserImpl : public Advertiser::Impl
    {
    public:
        BonjourAdvertiserImpl() = default;

        Error registerService(const std::string& serviceName, uint16_t port) noexcept override;
        Error registerService(const std::string& serviceName, uint16_t port, const TXTRecord& txtRecord) noexcept override;
        Error registerService(const std::string& serviceName,
                              uint16_t port,
                              const std::map<std::string, std::string>& keysValues) noexcept override;

        void unregisterService() noexcept override;
        void callObserver(std::function<void(Advertiser::Observer&)>) noexcept;

    private:
        DNSServiceRef mServiceRef = nullptr;
        Advertiser::Observer* mObserver = nullptr;
    };
}
