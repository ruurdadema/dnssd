#pragma once

#include "../common/TxtRecord.h"

namespace dnssd {

    class CommonAdvertiserInterface
    {
    public:
        explicit CommonAdvertiserInterface() = default;
        virtual ~CommonAdvertiserInterface() = default;

        std::function<void(const Error&)> onAdvertiserErrorAsync;

        virtual Error registerService(const std::string& serviceName, uint16_t port) noexcept = 0;
        virtual Error registerService(const std::string& serviceName, uint16_t port, const TxtRecord& txtRecord) noexcept = 0;
        virtual void unregisterService() noexcept = 0;
    };

} // namespace dnssd
