#pragma once

#include "../common/CommonBrowserInterface.h"
#include "../common/Error.h"
#include "../common/Log.h"

#include "Service.h"
#include "SharedConnection.h"

#include <atomic>
#include <thread>
#include <mutex>

namespace dnssd
{

class BonjourBrowser : public CommonBrowserInterface
{
public:
    explicit BonjourBrowser();
    ~BonjourBrowser() override;

    Error browseFor (const std::string& service) override;

    bool reportIfError (const Error& error) noexcept;

    void browseReply (
        DNSServiceRef browseServiceRef,
        DNSServiceFlags inFlags,
        uint32_t interfaceIndex,
        DNSServiceErrorType errorCode,
        const char* name,
        const char* type,
        const char* domain);

    const SharedConnection& sharedConnection() const noexcept { return mSharedConnection; }

private:
    SharedConnection mSharedConnection;
    std::map<std::string, ScopedDnsServiceRef> mBrowsers;
    std::map<std::string, Service> mServices;
    std::atomic_bool mKeepGoing = ATOMIC_VAR_INIT (true);
    std::thread mThread;
    std::recursive_mutex mLock;

    void thread();
};

} // namespace dnssd
