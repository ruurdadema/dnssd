#include "dnssd/windns/WindnsBrowser.h"
#include "dnssd/windns/WindnsUtil.h"

namespace
{

/**
 * Splits a fully-qualified service instance name into its components.
 *
 * Expects the form "<name>.<type>.<domain>" where <type> has exactly two DNS labels
 * (e.g. "_http._tcp") and <domain> is "local". Works backwards from ".local" so that
 * service names containing "._" are handled correctly.
 *
 * @param instanceName  Instance name as received from DnsServiceBrowse (e.g. "My Device._http._tcp.local").
 * @param name          Receives the service instance name (e.g. "My Device").
 * @param type          Receives the service type with trailing dot (e.g. "_http._tcp.").
 * @param domain        Receives the domain with trailing dot (e.g. "local.").
 * @return true on success, false if the name cannot be parsed.
 */
bool parseInstanceName (
    const std::string& instanceName,
    std::string& name,
    std::string& type,
    std::string& domain)
{
    const std::string localSuffix = ".local";
    const auto localPos = instanceName.rfind (localSuffix);
    if (localPos == std::string::npos || localPos == 0)
    {
        DNSSD_LOG_DEBUG ("instance name does not end in .local: " << instanceName << std::endl);
        return false;
    }

    // Everything before ".local": "<name>._type._proto"
    const std::string nameAndType = instanceName.substr (0, localPos);

    // Find the 2nd dot from the right — that is where the name ends and the type begins.
    // Using rfind twice handles service names that themselves contain "._".
    auto dotPos = nameAndType.rfind ('.');
    if (dotPos == std::string::npos || dotPos == 0)
        return false;
    dotPos = nameAndType.rfind ('.', dotPos - 1);
    if (dotPos == std::string::npos)
        return false;

    name = nameAndType.substr (0, dotPos);
    type = nameAndType.substr (dotPos + 1) + "."; // e.g. "_http._tcp."
    domain = "local.";
    return true;
}

/**
 * @return true if str ends with suffix, ignoring a single trailing dot on str.
 */
template <typename StringType>
bool endsWithDomain (StringType str, const StringType& suffix)
{
    if (!str.empty() && str.back() == '.')
        str.pop_back();
    return str.size() >= suffix.size() && str.compare (str.size() - suffix.size(), suffix.size(), suffix) == 0;
}

/**
 * @return true for statuses that indicate a query simply yielded no (more) results, as opposed to a real error.
 * These are expected for mDNS, e.g. an AAAA query for a host without IPv6 address or a resolve for a service which
 * disappeared without sending a goodbye packet.
 */
bool isNoResultStatus (DNS_STATUS status)
{
    return status == DNS_INFO_NO_RECORDS || status == DNS_ERROR_RCODE_NAME_ERROR || status == ERROR_TIMEOUT;
}

} // namespace

namespace dnssd
{

void WindnsBrowser::setTxtPollIntervalMs (uint32_t intervalMs)
{
    mTxtPollIntervalMs.store (intervalMs, std::memory_order_release);

    // Start thread if not already running, otherwise the running thread picks up the new interval.
    if (intervalMs > 0)
        startPollThread();
    else
        stopPollThread();
}

void WindnsBrowser::startPollThread()
{
    std::lock_guard<std::mutex> lg (mPollThreadMutex);

    if (mPollThread.joinable() || mTxtPollIntervalMs.load (std::memory_order_acquire) == 0)
        return;

    {
        std::lock_guard<std::mutex> lk (mPollMutex);
        mStopPollThread = false;
    }
    mPollThread = std::thread (&WindnsBrowser::pollLoop, this);
}

void WindnsBrowser::stopPollThread()
{
    std::lock_guard<std::mutex> lg (mPollThreadMutex);

    if (!mPollThread.joinable())
        return;

    {
        // Set the flag under the mutex so the wakeup can't be missed between the predicate check and the wait.
        std::lock_guard<std::mutex> lk (mPollMutex);
        mStopPollThread = true;
    }
    mPollCv.notify_one();
    mPollThread.join();
}

void WindnsBrowser::pollLoop()
{
    while (true)
    {
        {
            std::unique_lock<std::mutex> lk (mPollMutex);
            const auto interval = std::chrono::milliseconds (mTxtPollIntervalMs.load (std::memory_order_acquire));
            mPollCv.wait_for (lk, interval, [this]() { return mStopPollThread; });
            if (mStopPollThread)
                break;
        }

        std::lock_guard<std::recursive_mutex> lg (mLock);
        if (mShuttingDown)
            break;

        for (auto& entry : mServices)
            issueResolve (entry.first, entry.second);
    }
}

void WindnsBrowser::operationStarted()
{
    std::lock_guard<std::mutex> lk (mPendingMutex);
    ++mPendingOperations;
}

void WindnsBrowser::operationFinished()
{
    // Notify while holding the mutex: as soon as it is released the destructor may complete.
    std::lock_guard<std::mutex> lk (mPendingMutex);
    if (--mPendingOperations == 0)
        mPendingCv.notify_all();
}

WindnsBrowser::~WindnsBrowser()
{
    stopPollThread();

    {
        std::lock_guard<std::recursive_mutex> lg (mLock);
        mShuttingDown = true;

        // Cancelling invokes the completion callbacks with ERROR_CANCELLED (synchronously, on this thread), which
        // finishes the pending operations.
        for (auto& entry : mServices)
        {
            WindnsDiscoveredService& service = entry.second;
            if (service.resolveInFlight)
            {
                DWORD res = DnsServiceResolveCancel (&service.resolveCancel);
                if (res != ERROR_SUCCESS && res != ERROR_CANCELLED)
                    DNSSD_LOG_DEBUG ("WindnsBrowser::~WindnsBrowser: DnsServiceResolveCancel failed: "
                                     << Result (static_cast<DNS_STATUS> (res)).description() << std::endl)
            }
            if (service.aQueryInFlight)
            {
                DWORD res = DnsCancelQuery (&service.aCancel);
                if (res != ERROR_SUCCESS && res != ERROR_CANCELLED)
                    DNSSD_LOG_DEBUG ("WindnsBrowser::~WindnsBrowser: DnsCancelQuery (A) failed: "
                                     << Result (static_cast<DNS_STATUS> (res)).description() << std::endl)
            }
            if (service.aaaaQueryInFlight)
            {
                DWORD res = DnsCancelQuery (&service.aaaaCancel);
                if (res != ERROR_SUCCESS && res != ERROR_CANCELLED)
                    DNSSD_LOG_DEBUG ("WindnsBrowser::~WindnsBrowser: DnsCancelQuery (AAAA) failed: "
                                     << Result (static_cast<DNS_STATUS> (res)).description() << std::endl)
            }
        }

        for (auto& entry : mBrowseCancels)
        {
            DWORD res = DnsServiceBrowseCancel (&entry.second);
            if (res != ERROR_SUCCESS && res != ERROR_CANCELLED)
                DNSSD_LOG_DEBUG ("WindnsBrowser::~WindnsBrowser: DnsServiceBrowseCancel failed: "
                                 << Result (static_cast<DNS_STATUS> (res)).description() << std::endl)
        }
    }

    // Wait for all callbacks to finish. The cancel handles in mServices and mBrowseCancels must stay valid until then.
    std::unique_lock<std::mutex> lk (mPendingMutex);
    bool completed = mPendingCv.wait_for (lk, kWindnsCallbackTimeout, [this]() { return mPendingOperations == 0; });
    if (!completed)
        DNSSD_LOG_DEBUG ("WindnsBrowser::~WindnsBrowser: timed out waiting for " << mPendingOperations
                                                                                   << " pending callbacks" << std::endl)
}

Result WindnsBrowser::browseFor (const std::string& serviceType)
{
    std::wstring queryName;
    auto r = toWideString (serviceType, queryName);
    if (r.hasError())
        return r;

    // Normalize to the form "_type._proto.local" expected by DnsServiceBrowse:
    // strip any trailing dots, then append .local if not already present.
    while (!queryName.empty() && queryName.back() == L'.')
        queryName.pop_back();

    if (!endsWithDomain (queryName, std::wstring (L".local")))
        queryName += L".local";

    {
        std::lock_guard<std::recursive_mutex> lg (mLock);

        if (mBrowseCancels.count (queryName) > 0)
            return Result ("already browsing for service \"" + serviceType + "\"");

        DNS_SERVICE_BROWSE_REQUEST req {};
        req.Version = DNS_QUERY_REQUEST_VERSION1;
        req.InterfaceIndex = 0; // all interfaces
        req.QueryName = queryName.c_str();
        req.pBrowseCallback = &WindnsBrowser::browseCallback;
        req.pQueryContext = this;

        auto it = mBrowseCancels.emplace (queryName, DNS_SERVICE_CANCEL {}).first;

        // The browse callback is invoked one final time with ERROR_CANCELLED when the browse is cancelled.
        operationStarted();
        DWORD status = DnsServiceBrowse (&req, &it->second);

        if (status != DNS_REQUEST_PENDING)
        {
            operationFinished();
            mBrowseCancels.erase (it);
            return Result (static_cast<DNS_STATUS> (status));
        }
    }

    // Start outside of mLock: the poll thread acquires mLock while stopPollThread holds mPollThreadMutex.
    startPollThread();

    return {};
}

VOID WINAPI WindnsBrowser::browseCallback (DWORD Status, PVOID pQueryContext, PDNS_RECORD pDnsRecord)
{
    auto* self = static_cast<WindnsBrowser*> (pQueryContext);

    // Cancelling the browse request calls this callback one more time for cleanup.
    if (static_cast<DNS_STATUS> (Status) == ERROR_CANCELLED)
    {
        if (pDnsRecord != nullptr)
            DnsRecordListFree (pDnsRecord, DnsFreeRecordList);
        self->operationFinished();
        return;
    }

    {
        std::lock_guard<std::recursive_mutex> lg (self->mLock);

        DNSSD_LOG_DEBUG ("> browseCallback enter" << std::endl)

        if (!self->mShuttingDown && !self->reportIfError (Result (static_cast<DNS_STATUS> (Status))))
        {
            for (PDNS_RECORD record = pDnsRecord; record != nullptr; record = record->pNext)
            {
                if (record->wType != DNS_TYPE_PTR)
                    continue;

                if (record->Data.PTR.pNameHost == nullptr)
                {
                    DNSSD_LOG_DEBUG ("- browseCallback: host name string was nullptr" << std::endl);
                    continue;
                }

                std::string instanceName;
                if (toNarrowString (record->Data.PTR.pNameHost, instanceName).hasError())
                {
                    DNSSD_LOG_DEBUG ("- browseCallback: failed to convert instance name" << std::endl);
                    continue;
                }

                auto it = self->mServices.find (instanceName);

                if (record->dwTtl == 0)
                {
                    // Goodbye packet — the service is being removed
                    if (it == self->mServices.end())
                        continue;

                    WindnsDiscoveredService& service = it->second;

                    // These invoke the callbacks synchronously with ERROR_CANCELLED.
                    if (service.resolveInFlight)
                        DnsServiceResolveCancel (&service.resolveCancel);
                    if (service.aQueryInFlight)
                        DnsCancelQuery (&service.aCancel);
                    if (service.aaaaQueryInFlight)
                        DnsCancelQuery (&service.aaaaCancel);

                    DNSSD_LOG_DEBUG ("- browseCallback: removed " << service.description.fullname << std::endl)

                    if (self->onServiceRemovedCallback)
                        self->onServiceRemovedCallback (service.description);

                    self->mServices.erase (it);
                }
                else if (it == self->mServices.end())
                {
                    // First sighting — parse the instance name and create a new entry
                    std::string name, type, domain;
                    if (!parseInstanceName (instanceName, name, type, domain))
                    {
                        DNSSD_LOG_DEBUG ("- browseCallback: failed to parse instance name, skipping" << std::endl)
                        continue;
                    }

                    WindnsDiscoveredService service;
                    service.id = self->mNextServiceId++;
                    service.description.fullname = instanceName;
                    service.description.name = name;
                    service.description.type = type;
                    service.description.domain = domain;

                    it = self->mServices.emplace (instanceName, std::move (service)).first;

                    DNSSD_LOG_DEBUG ("- browseCallback: discovered " << instanceName << std::endl)

                    if (self->onServiceDiscoveredCallback)
                        self->onServiceDiscoveredCallback (it->second.description);

                    self->issueResolve (instanceName, it->second);
                }
                else
                {
                    // Subsequent sighting — re-resolve to pick up TXT record updates.
                    // Skipped by issueResolve if a resolve is already in flight.
                    self->issueResolve (instanceName, it->second);
                }
            }
        }

        DNSSD_LOG_DEBUG ("< browseCallback exit" << std::endl)
    }

    if (pDnsRecord != nullptr)
        DnsRecordListFree (pDnsRecord, DnsFreeRecordList);
}

VOID WINAPI WindnsBrowser::resolveCallback (DWORD Status, PVOID pQueryContext, PDNS_SERVICE_INSTANCE pInstance)
{
    auto* ctx = static_cast<ResolveContext*> (pQueryContext);
    auto* self = ctx->owner;
    const auto status = static_cast<DNS_STATUS> (Status);

    {
        std::lock_guard<std::recursive_mutex> lg (self->mLock);

        DNSSD_LOG_DEBUG ("> resolveCallback enter: " << ctx->instanceName << std::endl)

        auto it = self->findService (ctx->instanceName, ctx->serviceId);
        if (it == self->mServices.end())
        {
            DNSSD_LOG_DEBUG ("- resolveCallback: service already removed, discarding result" << std::endl)
        }
        else
        {
            it->second.resolveInFlight = false;

            if (status == ERROR_CANCELLED || isNoResultStatus (status))
            {
                DNSSD_LOG_DEBUG ("- resolveCallback: no result for " << ctx->instanceName << " (" << status << ")" << std::endl)
            }
            else if (self->reportIfError (Result (status)))
            {
                DNSSD_LOG_DEBUG ("- resolveCallback: resolve failed for " << ctx->instanceName << std::endl)
            }
            else if (pInstance == nullptr)
            {
                DNSSD_LOG_DEBUG ("- resolveCallback: pInstance is null" << std::endl)
            }
            else
            {
                self->onResolveResult (ctx->instanceName, it->second, pInstance);
            }
        }

        DNSSD_LOG_DEBUG ("< resolveCallback exit: " << ctx->instanceName << std::endl)
    }

    if (pInstance != nullptr)
        DnsServiceFreeInstance (pInstance);

    delete ctx;
    self->operationFinished();
}

VOID WINAPI WindnsBrowser::addressQueryCallback (PVOID pQueryContext, PDNS_QUERY_RESULT pQueryResults)
{
    auto* ctx = static_cast<AddressQueryContext*> (pQueryContext);
    auto* self = ctx->owner;

    {
        std::lock_guard<std::recursive_mutex> lg (self->mLock);

        DNSSD_LOG_DEBUG ("> addressQueryCallback enter: " << ctx->instanceName << " type " << ctx->queryType << std::endl)

        auto it = self->findService (ctx->instanceName, ctx->serviceId);
        if (it == self->mServices.end())
        {
            DNSSD_LOG_DEBUG ("- addressQueryCallback: service already removed, discarding" << std::endl)
        }
        else
        {
            if (ctx->queryType == DNS_TYPE_A)
                it->second.aQueryInFlight = false;
            else
                it->second.aaaaQueryInFlight = false;

            const auto status = pQueryResults != nullptr ? pQueryResults->QueryStatus : ERROR_INVALID_DATA;

            if (status == ERROR_SUCCESS)
                self->onAddressResult (it->second, pQueryResults->pQueryRecords, ctx->interfaceIndex);
            else if (status == ERROR_CANCELLED || isNoResultStatus (status))
                DNSSD_LOG_DEBUG ("- addressQueryCallback: no result for " << ctx->instanceName << " (" << status << ")" << std::endl)
            else
                self->reportIfError (Result (status));
        }

        DNSSD_LOG_DEBUG ("< addressQueryCallback exit: " << ctx->instanceName << std::endl)
    }

    if (pQueryResults != nullptr && pQueryResults->pQueryRecords != nullptr)
        DnsRecordListFree (pQueryResults->pQueryRecords, DnsFreeRecordList);

    delete ctx;
    self->operationFinished();
}

std::map<std::string, WindnsDiscoveredService>::iterator
WindnsBrowser::findService (const std::string& instanceName, uint64_t id)
{
    auto it = mServices.find (instanceName);
    if (it != mServices.end() && it->second.id != id)
        return mServices.end();
    return it;
}

void WindnsBrowser::issueResolve (const std::string& instanceName, WindnsDiscoveredService& service)
{
    if (mShuttingDown || service.resolveInFlight)
        return;

    std::wstring instanceNameW;
    auto r = toWideString (instanceName, instanceNameW);
    if (reportIfError (r))
        return;

    auto* ctx = new ResolveContext { this, instanceName, service.id };

    DNS_SERVICE_RESOLVE_REQUEST req {};
    req.Version = DNS_QUERY_REQUEST_VERSION1;
    req.InterfaceIndex = 0;
    req.QueryName = const_cast<PWSTR> (instanceNameW.c_str());
    req.pResolveCompletionCallback = &WindnsBrowser::resolveCallback;
    req.pQueryContext = ctx;

    operationStarted();
    service.resolveInFlight = true;

    DWORD status = DnsServiceResolve (&req, &service.resolveCancel);

    if (status != DNS_REQUEST_PENDING)
    {
        service.resolveInFlight = false;
        delete ctx;
        operationFinished();
        reportIfError (Result (static_cast<DNS_STATUS> (status)));
    }
}

void WindnsBrowser::issueAddressQueries (
    const std::string& instanceName,
    WindnsDiscoveredService& service,
    const std::string& hostName,
    uint32_t interfaceIndex)
{
    std::wstring hostNameW;
    auto r = toWideString (hostName, hostNameW);
    if (reportIfError (r))
        return;

    issueAddressQuery (instanceName, service, hostNameW, DNS_TYPE_A, interfaceIndex);
    issueAddressQuery (instanceName, service, hostNameW, DNS_TYPE_AAAA, interfaceIndex);
}

void WindnsBrowser::issueAddressQuery (
    const std::string& instanceName,
    WindnsDiscoveredService& service,
    const std::wstring& hostName,
    WORD queryType,
    uint32_t interfaceIndex)
{
    bool& inFlight = queryType == DNS_TYPE_A ? service.aQueryInFlight : service.aaaaQueryInFlight;
    DNS_QUERY_CANCEL& cancel = queryType == DNS_TYPE_A ? service.aCancel : service.aaaaCancel;

    if (mShuttingDown || inFlight)
        return;

    // .local hostnames must be resolved via multicast DNS; standard unicast DNS has no record of them.
    const DWORD queryOptions = endsWithDomain (hostName, std::wstring (L".local")) ? DNS_QUERY_MULTICAST_ONLY
                                                                                    : DNS_QUERY_STANDARD;

    auto* ctx = new AddressQueryContext { this, instanceName, service.id, queryType, interfaceIndex };
    ctx->queryResult.Version = DNS_QUERY_REQUEST_VERSION1;

    DNS_QUERY_REQUEST req {};
    req.Version = DNS_QUERY_REQUEST_VERSION1;
    req.QueryName = hostName.c_str();
    req.QueryType = queryType;
    req.QueryOptions = queryOptions;
    req.InterfaceIndex = 0;
    req.pQueryCompletionCallback = &WindnsBrowser::addressQueryCallback;
    req.pQueryContext = ctx;

    operationStarted();
    inFlight = true;

    DNS_STATUS status = DnsQueryEx (&req, &ctx->queryResult, &cancel);

    if (status == DNS_REQUEST_PENDING)
        return;

    // Completed synchronously (or failed); the callback will not be invoked.
    inFlight = false;
    operationFinished();

    if (status == ERROR_SUCCESS)
        onAddressResult (service, ctx->queryResult.pQueryRecords, interfaceIndex);
    else if (!isNoResultStatus (status))
        reportIfError (Result (status));

    if (ctx->queryResult.pQueryRecords != nullptr)
        DnsRecordListFree (ctx->queryResult.pQueryRecords, DnsFreeRecordList);
    delete ctx;
}

void WindnsBrowser::onResolveResult (
    const std::string& instanceName,
    WindnsDiscoveredService& service,
    PDNS_SERVICE_INSTANCE pInstance)
{
    ServiceDescription& desc = service.description;

    std::string hostTarget;
    if (pInstance->pszHostName != nullptr)
    {
        auto r = toNarrowString (pInstance->pszHostName, hostTarget);
        if (reportIfError (r))
            return;
    }

    TxtRecord txtRecord;
    for (DWORD i = 0; i < pInstance->dwPropertyCount; ++i)
    {
        if (pInstance->keys == nullptr || pInstance->keys[i] == nullptr)
            continue;

        std::string key, value;
        auto rKey = toNarrowString (pInstance->keys[i], key);
        if (rKey.hasError())
        {
            DNSSD_LOG_DEBUG ("- onResolveResult: failed to convert TXT key at index " << i << std::endl)
            continue;
        }

        if (pInstance->values != nullptr && pInstance->values[i] != nullptr)
        {
            auto rVal = toNarrowString (pInstance->values[i], value);
            if (rVal.hasError())
            {
                DNSSD_LOG_DEBUG ("- onResolveResult: failed to convert TXT value for key \"" << key << "\"" << std::endl)
                continue;
            }
        }

        txtRecord[key] = value;
    }

    const bool hostChanged = !service.resolved || hostTarget != desc.hostTarget;
    const bool changed = hostChanged || pInstance->wPort != desc.port || txtRecord != desc.txtRecord;

    desc.hostTarget = std::move (hostTarget);
    desc.port = pInstance->wPort;
    desc.txtRecord = std::move (txtRecord);
    service.resolved = true;

    const uint32_t ifIndex = pInstance->dwInterfaceIndex;

    // The TXT poll loop re-resolves periodically; only report when something actually changed.
    if (changed)
    {
        DNSSD_LOG_DEBUG ("- onResolveResult: " << instanceName << " host=" << desc.hostTarget << " port=" << desc.port << std::endl)

        if (onServiceResolvedCallback)
            onServiceResolvedCallback (desc, ifIndex);
    }

    // Bonjour includes A/AAAA records as additional records in the mDNS SRV response, which
    // DnsServiceResolve populates into ip4Address/ip6Address. Use them directly to avoid a
    // separate DnsQueryEx round-trip, which does not reliably resolve .local names via mDNS.
    bool hasAddresses = false;

    if (pInstance->ip4Address != nullptr)
    {
        char buf[INET_ADDRSTRLEN];
        if (inet_ntop (AF_INET, pInstance->ip4Address, buf, sizeof (buf)) != nullptr)
        {
            std::string address = buf;
            if (desc.interfaces[ifIndex].insert (address).second)
            {
                DNSSD_LOG_DEBUG ("- onResolveResult: added IPv4 " << address << " for " << instanceName << std::endl)
                if (onAddressAddedCallback)
                    onAddressAddedCallback (desc, address, ifIndex);
            }
            hasAddresses = true;
        }
    }

    if (pInstance->ip6Address != nullptr)
    {
        char buf[INET6_ADDRSTRLEN];
        if (inet_ntop (AF_INET6, pInstance->ip6Address, buf, sizeof (buf)) != nullptr)
        {
            std::string address = buf;
            if (desc.interfaces[ifIndex].insert (address).second)
            {
                DNSSD_LOG_DEBUG ("- onResolveResult: added IPv6 " << address << " for " << instanceName << std::endl)
                if (onAddressAddedCallback)
                    onAddressAddedCallback (desc, address, ifIndex);
            }
            hasAddresses = true;
        }
    }

    // Only fall back to address queries when needed, not on every poll.
    if (!hasAddresses && !desc.hostTarget.empty() && (hostChanged || desc.interfaces.empty()))
        issueAddressQueries (instanceName, service, desc.hostTarget, ifIndex);
}

void WindnsBrowser::onAddressResult (WindnsDiscoveredService& service, PDNS_RECORD records, uint32_t interfaceIndex)
{
    ServiceDescription& desc = service.description;

    for (PDNS_RECORD record = records; record != nullptr; record = record->pNext)
    {
        std::string address;

        if (record->wType == DNS_TYPE_A)
        {
            char buf[INET_ADDRSTRLEN];
            DWORD ipAddr = record->Data.A.IpAddress;
            if (inet_ntop (AF_INET, &ipAddr, buf, sizeof (buf)) == nullptr)
                continue;
            address = buf;
        }
        else if (record->wType == DNS_TYPE_AAAA)
        {
            char buf[INET6_ADDRSTRLEN];
            if (inet_ntop (AF_INET6, &record->Data.AAAA.Ip6Address, buf, sizeof (buf)) == nullptr)
                continue;
            address = buf;
        }
        else
        {
            continue;
        }

        // Goodbye message for a specific interface
        if (record->dwTtl == 0)
        {
            auto ifIt = desc.interfaces.find (interfaceIndex);
            if (ifIt != desc.interfaces.end() && ifIt->second.erase (address) > 0)
            {
                if (ifIt->second.empty())
                    desc.interfaces.erase (ifIt);

                DNSSD_LOG_DEBUG ("- onAddressResult: removed " << address << " for " << desc.fullname << std::endl)

                if (onAddressRemovedCallback)
                    onAddressRemovedCallback (desc, address, interfaceIndex);
            }
        }
        else
        {
            bool inserted = desc.interfaces[interfaceIndex].insert (address).second;
            if (inserted)
            {
                DNSSD_LOG_DEBUG ("- onAddressResult: added " << address << " for " << desc.fullname << std::endl)

                if (onAddressAddedCallback)
                    onAddressAddedCallback (desc, address, interfaceIndex);
            }
        }
    }
}

bool WindnsBrowser::reportIfError (const Result& result) const noexcept
{
    if (result.hasError())
    {
        if (onBrowseErrorCallback)
            onBrowseErrorCallback (result);
        return true;
    }
    return false;
}

} // namespace dnssd
