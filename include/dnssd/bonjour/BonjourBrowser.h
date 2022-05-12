#pragma once

#include "../common/Result.h"
#include "../common/IBrowser.h"
#include "../common/Log.h"

#include "Service.h"
#include "SharedConnection.h"

#include <atomic>
#include <mutex>
#include <thread>

namespace dnssd
{

/**
 * Apple Bonjour implementation of IBrowser. Works on macOS and Windows.
 */
class BonjourBrowser : public IBrowser
{
public:
    explicit BonjourBrowser();
    ~BonjourBrowser() override;

    // MARK: IBrowser implementations -
    Result browseFor (const std::string& service) override;
    bool reportIfError (const Result& result) noexcept;

    /**
     * Called by dns_sd logic in response to a browse reply.
     * @param browseServiceRef The DNSServiceRef.
     * @param inFlags Possible values are kDNSServiceFlagsMoreComing and kDNSServiceFlagsAdd.
     *                See flag definitions for details.
     * @param interfaceIndex The interface on which the service is advertised. This index should be passed to
     *                       DNSServiceResolve() when resolving the service.
     * @param errorCode Will be kDNSServiceErr_NoError (0) on success, otherwise will indicate the failure that
     *                  occurred. Other parameters are undefined if the errorCode is nonzero.
     * @param name The discovered service name. This name should be displayed to the user, and stored for subsequent use
     *             in the DNSServiceResolve() call.
     * @param type The service type.
     * @param domain The domain of the discovered service instance.
     */
    void browseReply (
        DNSServiceRef browseServiceRef,
        DNSServiceFlags inFlags,
        uint32_t interfaceIndex,
        DNSServiceErrorType errorCode,
        const char* name,
        const char* type,
        const char* domain);

    /**
     * @return Returns the SharedConnection this instance is using for communicating with the mdns responder.
     */
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
