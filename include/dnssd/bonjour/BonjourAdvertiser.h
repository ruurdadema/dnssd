//
// Created by Ruurd Adema on 04/07/2020.
//

#pragma once

#include "../common/Error.h"
#include "../common/CommonAdvertiserInterface.h"
#include "ScopedDNSServiceRef.h"

#include <map>

namespace dnssd {

    class BonjourAdvertiser : public CommonAdvertiserInterface
    {
    public:
        using Listener = CommonAdvertiserInterface::Listener;

        explicit BonjourAdvertiser(const Listener& listener);

        Error registerService(const std::string& serviceName, uint16_t port) noexcept override;
        Error registerService(const std::string& serviceName, uint16_t port, const TxtRecord& txtRecord) noexcept override;
        Error registerService(const std::string& serviceName,
                              uint16_t port,
                              const std::map<std::string, std::string>& keysValues) noexcept override;

        void unregisterService() noexcept override;

    private:
        ScopedDNSServiceRef mServiceRef;
    };

} // namespace dnssd
