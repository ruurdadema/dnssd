#pragma once

#include <stdint.h>
#include <string>
#include <map>
#include <functional>

#include "dnssd/common/Error.h"
#include "dnssd/common/AdvertiserInterface.h"
#include "dnssd/bonjour/TXTRecord.h"

namespace dnssd {

    class Advertiser
    {
    public:
        using Listener = AdvertiserInterface::Listener;

        explicit Advertiser(const Listener& listener);
        ~Advertiser();

        Error registerService(const std::string& serviceName, uint16_t port) noexcept;
        Error registerService(const std::string& serviceName, uint16_t port, const TXTRecord& txtRecord) noexcept;
        Error registerService(const std::string& serviceName, uint16_t port, const std::map<std::string, std::string>& keysValues) noexcept;

    private:
        std::unique_ptr<AdvertiserInterface> impl;
    };

} // namespace dnssd
