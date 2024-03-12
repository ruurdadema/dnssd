#pragma once

#include "../common/AdvertiserBase.h"
#include "../common/Result.h"
#include "ScopedDnsServiceRef.h"

#include <map>
#include <mutex>

namespace dnssd
{

/**
 * Apple Bonjour implementation of IAdvertiser. Works on macOS and Windows.
 */
class BonjourAdvertiser : public AdvertiserBase
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

    static void DNSSD_API registerServiceCallBack (
        DNSServiceRef serviceRef,
        DNSServiceFlags flags,
        DNSServiceErrorType errorCode,
        const char* serviceName,
        const char* regType,
        const char* replyDomain,
        void* context);
};

} // namespace dnssd
