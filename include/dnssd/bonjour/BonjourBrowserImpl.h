//
// Created by Ruurd Adema on 04/07/2020.
//

#pragma once

#include "../internal/Debug.h"
#include "../internal/Error.h"
#include "../Browser.h"

#include "SharedConnection.h"

namespace dnssd
{
    class BonjourBrowserImpl : public Browser::Impl
    {
    public:

        BonjourBrowserImpl(const Browser::Listener& listener);
        ~BonjourBrowserImpl();

        Error browseFor(const std::string& service);

        [[nodiscard]] bool reportIfError(const Error& error) const noexcept;

        void callListener(const std::function<void(const Browser::Listener&)>&) const noexcept;

        void browseReply(DNSServiceRef browseServiceRef, DNSServiceFlags inFlags, uint32_t interfaceIndex,
                         DNSServiceErrorType errorCode, const char* name, const char* type,
                         const char* domain);

        [[nodiscard]] const SharedConnection& sharedConnection() const noexcept { return mSharedConnection; }

    private:
        SharedConnection mSharedConnection;
        const Browser::Listener& mListener;
        std::map<std::string, ScopedDNSServiceRef> mBrowsers;
        std::map<std::string, Service> mServices;
        std::atomic_bool mKeepGoing = ATOMIC_VAR_INIT(true);
        std::thread mThread;

        void thread();
    };
} // namespace dnssd
