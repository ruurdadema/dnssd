#pragma once

#include "../common/TxtRecord.h"
#include "Error.h"
#include "Util.h"

#include <functional>

namespace dnssd
{

class CommonAdvertiserInterface
{
public:
    explicit CommonAdvertiserInterface() = default;
    virtual ~CommonAdvertiserInterface() = default;

    virtual Error registerService (const std::string& serviceName, uint16_t port, const char* name) noexcept = 0;
    virtual Error registerService (
        const std::string& serviceName,
        uint16_t port,
        const TxtRecord& txtRecord,
        const char* name) noexcept = 0;
    virtual Error updateTxtRecord (const TxtRecord& txtRecord) = 0;
    virtual void unregisterService() noexcept = 0;

    virtual void onAdvertiserErrorAsync (const Error& error) { ignore (error); }
};

} // namespace dnssd
