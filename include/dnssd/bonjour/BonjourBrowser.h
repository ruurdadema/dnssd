#pragma once

#include "../common/Log.h"
#include "../common/Error.h"
#include "../common/CommonBrowserInterface.h"

#include "SharedConnection.h"
#include "Service.h"

#include <thread>

namespace dnssd {

    class BonjourBrowser : public CommonBrowserInterface
    {
    public:
        explicit BonjourBrowser();
        ~BonjourBrowser() override;

        Error browseFor(const std::string& service) override;

        [[nodiscard]] bool reportIfError(const Error& error) const noexcept;


        void browseReply(DNSServiceRef browseServiceRef, DNSServiceFlags inFlags, uint32_t interfaceIndex,
                         DNSServiceErrorType errorCode, const char* name, const char* type,
                         const char* domain);

        [[nodiscard]] const SharedConnection& sharedConnection() const noexcept { return mSharedConnection; }

    private:
        SharedConnection mSharedConnection;
        std::map<std::string, ScopedDnsServiceRef> mBrowsers;
        std::map<std::string, Service> mServices;
        std::atomic_bool mKeepGoing = ATOMIC_VAR_INIT(true);
        std::thread mThread;

        void thread();
    };

} // namespace dnssd
