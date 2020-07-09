#pragma once

#include "../common/Error.h"
#include "../common/CommonAdvertiserInterface.h"
#include "ScopedDnsServiceRef.h"

#include <map>

namespace dnssd {

    class BonjourAdvertiser : public CommonAdvertiserInterface
    {
    public:
        explicit BonjourAdvertiser() = default;

        Error registerService(const std::string& serviceName, uint16_t port) noexcept override;
        Error registerService(const std::string& serviceName, uint16_t port, const TxtRecord& txtRecord) noexcept override;

        void unregisterService() noexcept override;

    private:
        ScopedDnsServiceRef mServiceRef;
    };

} // namespace dnssd
