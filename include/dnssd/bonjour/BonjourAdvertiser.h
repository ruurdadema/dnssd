#pragma once

#include "../common/IAdvertiser.h"
#include "../common/Result.h"
#include "ScopedDnsServiceRef.h"

#include <map>
#include <mutex>

namespace dnssd
{

/**
 * Apple Bonjour implementation of IAdvertiser. Works on macOS and Windows.
 */
class BonjourAdvertiser : public IAdvertiser
{
public:
    explicit BonjourAdvertiser() = default;

    // MARK: IAdvertiser implementations -
    Result registerService (
        const std::string& regType,
        const char* name,
        const char* domain,
        uint16_t port,
        const TxtRecord& txtRecord) noexcept override;

    Result updateTxtRecord (const TxtRecord& txtRecord) override;
    void unregisterService() noexcept override;

private:
    ScopedDnsServiceRef mServiceRef;
};

} // namespace dnssd
