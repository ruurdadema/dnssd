#pragma once

#include "../common/TxtRecord.h"
#include "Error.h"

#include <functional>

namespace dnssd
{

class CommonAdvertiserInterface
{
public:
    explicit CommonAdvertiserInterface() = default;
    virtual ~CommonAdvertiserInterface() = default;

    std::function<void (const Error&)> onAdvertiserErrorAsync;

    virtual Error registerService (const std::string& serviceName, uint16_t port, const char* name) noexcept = 0;
    virtual Error registerService (
        const std::string& serviceName,
        uint16_t port,
        const TxtRecord& txtRecord,
        const char* name) noexcept = 0;
    virtual Error updateTxtRecord (const TxtRecord& txtRecord) = 0;
    virtual void unregisterService() noexcept = 0;
};

} // namespace dnssd
