#pragma once

#if !_WIN32 || !USE_WINDNS
#error This file should only be included on Windows builds and have the 'USE_WINDNS' compilation flag enabled.
#endif

#include "Windns.h"
#include "WindnsDiscoveredService.h"
#include "dnssd/common/BrowserBase.h"
#include "dnssd/common/Log.h"
#include "dnssd/common/Result.h"

#include <atomic>
#include <condition_variable>
#include <map>
#include <mutex>
#include <string>
#include <thread>

namespace dnssd
{

/**
 * Windows native DNS-SD implementation of BrowserBase using the WinDNS API (windns.h).
 * Only the .local mDNS domain is supported; custom domains are not available via this API.
 *
 * Discovery is performed in async stages:
 *   1. DnsServiceBrowse   — finds PTR records, fires onServiceDiscovered / onServiceRemoved
 *   2. DnsServiceResolve  — resolves host, port, TXT, and (if included) addresses via ip4Address /
 *                           ip6Address on DNS_SERVICE_INSTANCE; fires onServiceResolved and onAddressAdded
 *   3. DnsQueryEx (A/AAAA) — fallback address query when DnsServiceResolve does not include addresses
 *
 * All WinDNS callbacks arrive on thread-pool threads (or synchronously on the calling thread when a request is
 * cancelled); mLock serialises access to mServices.
 */
class WindnsBrowser : public BrowserBase
{
public:
    explicit WindnsBrowser() = default;

    /**
     * Sets up the browser with a custom TXT polling interval, if the interval is 0 the TXT polling loop
     * is not started - which can save CPU and network traffic if you don't need TXT record updates.
     *
     * @param txtPollIntervalMs - the interval in milliseconds that the TXT polling background thread uses.
     */
    explicit WindnsBrowser (uint32_t txtPollIntervalMs) : mTxtPollIntervalMs (txtPollIntervalMs) {}

    ~WindnsBrowser() override;

    /**
     * Starts browsing for a service type on the local mDNS domain.
     * Can be called multiple times with different service types.
     * Also starts the TXT polling thread, unless the poll interval is 0.
     * @param serviceType The service type (e.g. "_http._tcp.").
     * @return A Result indicating success or failure.
     */
    Result browseFor (const std::string& serviceType) override;

    /**
     * Sets the interval of the periodic re-resolution of discovered services, used to detect TXT record updates.
     * The WinDNS API has no record-change subscription, so polling is the only reliable way
     * to pick up TXT changes announced by remote peers (e.g. Bonjour on macOS).
     * Each poll re-issues DnsServiceResolve for every known service and fires
     * onServiceResolved if the host, port or TXT record has changed.
     *
     * You can safely call this method to update the interval for a background thread that's already running.
     * Either the atomic interval variable is updated and the thread picks it up on the next iteration,
     * or the thread is torn down when you pass 0 as an interval.
     * Don't call this method from within one of the browser callbacks.
     *
     * @param intervalMs Poll interval in milliseconds. Pass 0 to disable or to stop a running thread.
     */
    void setTxtPollIntervalMs (uint32_t intervalMs);

private:
    std::thread mPollThread;
    std::mutex mPollThreadMutex; // guards starting and stopping mPollThread
    std::condition_variable mPollCv;
    std::mutex mPollMutex; // guards mStopPollThread for mPollCv
    std::atomic<uint32_t> mTxtPollIntervalMs { 1000 };
    bool mStopPollThread = false;

    /// One cancel handle per browsed service type, keyed by wide-string service type.
    std::map<std::wstring, DNS_SERVICE_CANCEL> mBrowseCancels;

    /// All currently known service instances, keyed by fully-qualified instance name (narrow, as received from browse).
    std::map<std::string, WindnsDiscoveredService> mServices;

    /// Id given to the next entry in mServices.
    uint64_t mNextServiceId = 1;

    /// Set by the destructor; prevents new async operations from being issued.
    bool mShuttingDown = false;

    std::recursive_mutex mLock;

    /// Number of async WinDNS operations whose callback has not yet finished. The destructor waits for this to reach
    /// zero, so that no callback can access this object after it has been destroyed.
    int mPendingOperations = 0;
    std::mutex mPendingMutex;
    std::condition_variable mPendingCv;

    // -- Context structs passed as pQueryContext to WinDNS async calls --
    // Browse passes 'this' directly; resolve and address queries need the instance name too.

    struct ResolveContext
    {
        WindnsBrowser* owner;
        std::string instanceName;
        uint64_t serviceId;
    };

    struct AddressQueryContext
    {
        WindnsBrowser* owner;
        std::string instanceName;
        uint64_t serviceId;
        WORD queryType;
        uint32_t interfaceIndex;
        /// Embedded result struct kept alive until the async callback fires (pQueryResults == &queryResult).
        DNS_QUERY_RESULT queryResult {};
    };

    // -- Static WINAPI callbacks (invoked on WinDNS thread-pool threads) --

    /**
     * Fired by DnsServiceBrowse when PTR records are discovered or removed.
     * @param Status        DNS status code (ERROR_SUCCESS on success).
     * @param pQueryContext Pointer to the WindnsBrowser instance (passed as 'this' in browseFor).
     * @param pDnsRecord    Chain of DNS_RECORD results (DNS_TYPE_PTR). Must be freed with DnsRecordListFree.
     */
    static VOID WINAPI browseCallback (DWORD Status, PVOID pQueryContext, PDNS_RECORD pDnsRecord);

    /**
     * Fired by DnsServiceResolve when a service instance is resolved to its host, port, and TXT record.
     * Routes into onResolveResult() after casting pQueryContext to ResolveContext.
     * @param Status        DNS status code (ERROR_SUCCESS on success).
     * @param pQueryContext Pointer to a heap-allocated ResolveContext.
     * @param pInstance     Resolved service instance. Must be freed with DnsServiceFreeInstance.
     */
    static VOID WINAPI resolveCallback (DWORD Status, PVOID pQueryContext, PDNS_SERVICE_INSTANCE pInstance);

    /**
     * Fired by DnsQueryEx for A (IPv4) and AAAA (IPv6) record queries.
     * Routes into onAddressResult() after casting pQueryContext to AddressQueryContext.
     * @param pQueryContext Pointer to a heap-allocated AddressQueryContext.
     * @param pQueryResults Query results; pQueryResults->pQueryRecords is the DNS_RECORD chain.
     */
    static VOID WINAPI addressQueryCallback (PVOID pQueryContext, PDNS_QUERY_RESULT pQueryResults);

    // -- Instance methods (called from callbacks, mLock already held) --

    /**
     * Finds the service with the given name, but only if it still is the entry with the given id.
     * Must be called with mLock held.
     */
    std::map<std::string, WindnsDiscoveredService>::iterator findService (const std::string& instanceName, uint64_t id);

    /**
     * Issues DnsServiceResolve for the given service, unless a resolve is already in flight.
     * Stores the cancel handle in service.resolveCancel.
     * Must be called with mLock held.
     * @param instanceName Fully-qualified instance name (e.g. "MyDevice._http._tcp.local").
     * @param service      The entry in mServices for this instance name.
     */
    void issueResolve (const std::string& instanceName, WindnsDiscoveredService& service);

    /**
     * Issues DnsQueryEx for A and AAAA records for the given host name, unless already in flight.
     * Must be called with mLock held.
     * @param instanceName   Instance name used to look up the service on callback.
     * @param service        The entry in mServices for this instance name.
     * @param hostName       Host to query (e.g. "MyDevice.local").
     * @param interfaceIndex Interface index on which the service was resolved.
     */
    void issueAddressQueries (
        const std::string& instanceName,
        WindnsDiscoveredService& service,
        const std::string& hostName,
        uint32_t interfaceIndex);

    /**
     * Issues a single DnsQueryEx call for the given record type (DNS_TYPE_A or DNS_TYPE_AAAA).
     * Must be called with mLock held.
     */
    void issueAddressQuery (
        const std::string& instanceName,
        WindnsDiscoveredService& service,
        const std::wstring& hostName,
        WORD queryType,
        uint32_t interfaceIndex);

    /**
     * Processes a resolved DNS_SERVICE_INSTANCE, populating hostTarget, port, and txtRecord
     * on the matching service and firing onServiceResolvedCallback if anything changed.
     * Then adds the addresses contained in the instance, or issues A/AAAA address queries if there are none.
     * Must be called with mLock held.
     * @param instanceName Instance name.
     * @param service      The entry in mServices for this instance name.
     * @param pInstance    Resolved instance data. Caller is responsible for calling DnsServiceFreeInstance.
     */
    void onResolveResult (const std::string& instanceName, WindnsDiscoveredService& service, PDNS_SERVICE_INSTANCE pInstance);

    /**
     * Processes a chain of A or AAAA DNS_RECORD results, inserting new addresses into
     * description.interfaces and firing onAddressAddedCallback for each new address.
     * Zero-TTL records trigger removal from the set and fire onAddressRemovedCallback.
     * Must be called with mLock held.
     * @param service        The service the records belong to.
     * @param records        Chain of DNS_RECORD results (DNS_TYPE_A or DNS_TYPE_AAAA).
     * @param interfaceIndex Interface index to file the addresses under.
     */
    void onAddressResult (WindnsDiscoveredService& service, PDNS_RECORD records, uint32_t interfaceIndex);

    /**
     * Reports an error via onBrowseErrorCallback if result.hasError().
     * @return true if there was an error, false otherwise.
     */
    bool reportIfError (const Result& result) const noexcept;

    /// Registers an async operation whose callback is still to come. Must be called before issuing the request.
    void operationStarted();

    /// Marks an async operation as finished. Must be the very last thing a callback does with this object.
    void operationFinished();

    /// Starts the poll thread if it is not running and the poll interval is non-zero.
    void startPollThread();

    /// Stops the poll thread if it is running.
    void stopPollThread();

    /**
     * Background thread body for TXT polling. Sleeps for mTxtPollIntervalMs between each
     * round of re-resolves; wakes early when mStopPollThread is set.
     */
    void pollLoop();
};

} // namespace dnssd
