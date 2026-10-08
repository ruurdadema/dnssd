#pragma once

#if !_WIN32 || !USE_WINDNS
#error This file should only be included on Windows builds and have the 'USE_WINDNS' compilation flag enabled.
#endif

#include "Windns.h"
#include "dnssd/common/AdvertiserBase.h"
#include "dnssd/common/Result.h"
#include "dnssd/common/Log.h"

#include <condition_variable>
#include <mutex>
#include <string>

namespace dnssd
{

/**
 * Windows native DNS-SD implementation of AdvertiserBase using the WinDNS API (windns.h).
 * Only the .local mDNS domain is supported.
 *
 * Registration and deregistration complete asynchronously on a WinDNS thread-pool thread. The public methods
 * synchronise with these completions, so they can be called in any order (e.g. updateTxtRecord or unregisterService
 * directly after registerService).
 */
class WindnsAdvertiser : public AdvertiserBase
{
public:
    explicit WindnsAdvertiser() = default;
    ~WindnsAdvertiser() override;

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
    enum class State
    {
        Idle,          // Nothing registered
        Registering,   // DnsServiceRegister issued, waiting for the completion callback
        Registered,    // Registration completed successfully
        Deregistering, // DnsServiceDeRegister issued, waiting for the completion callback
    };

    DNS_SERVICE_REGISTER_REQUEST mReq {};
    DNS_SERVICE_CANCEL mCancel {};
    std::wstring mInstanceName;
    uint16_t mPort = 0;

    State mState = State::Idle; // Guarded by mLock
    std::mutex mLock;           // For synchronisation with the callback
    std::condition_variable mCv;

    static VOID WINAPI registerCompleteCallback (DWORD Status, PVOID pQueryContext, PDNS_SERVICE_INSTANCE pInstance);

    /**
     * Issues DnsServiceRegister for mInstanceName and mPort with the given TXT record.
     * Must be called while in the Idle state.
     */
    Result doRegister (const TxtRecord& txtRecord);

    /**
     * Waits until the state is no longer equal to the given state, or until kWindnsCallbackTimeout has passed.
     * @return true if the state changed, false on timeout.
     */
    bool waitWhileInState (std::unique_lock<std::mutex>& lock, State state);
};

} // namespace dnssd
