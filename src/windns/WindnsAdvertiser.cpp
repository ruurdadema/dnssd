#include "dnssd/windns/WindnsAdvertiser.h"
#include "dnssd/windns/WindnsUtil.h"

#include <sstream>
#include <string>
#include <vector>

namespace {

/**
 * Retrieves the DNS host name of this computer (without domain).
 * @param out Receives the host name on success.
 * @return Empty Result on success, or an error Result on failure.
 */
dnssd::Result getDnsHostname (std::wstring& out)
{
    DWORD bufferSize = 0;
    GetComputerNameExW (ComputerNameDnsHostname, nullptr, &bufferSize);
    if (GetLastError() != ERROR_MORE_DATA)
    {
        std::stringstream msg;
        msg << "Failed to get hostname size. Error code: " << GetLastError();
        return dnssd::Result (msg.str());
    }

    out.resize (bufferSize);
    if (!GetComputerNameExW (ComputerNameDnsHostname, &out[0], &bufferSize))
    {
        std::stringstream msg;
        msg << "Failed to get hostname. Error code: " << GetLastError();
        return dnssd::Result (msg.str());
    }
    out.resize (bufferSize); // trim null terminator written by the API
    return {};
}

} // namespace

namespace dnssd
{

Result WindnsAdvertiser::registerService (
    const std::string& regType,
    const char* name,
    const char* domain,
    uint16_t port,
    const TxtRecord& txtRecord) noexcept
{
    if (domain != nullptr)
        return Result ("Windns does not support other domain names than '.local', you need to leave the domain parameter as a nullptr");

    {
        std::lock_guard<std::mutex> lock (mLock);
        if (mState != State::Idle)
            return Result ("Service already registered, unregister first or use updateTxtRecord function to update an existing service");
    }

    // Build the fully qualified instance name: "<name>.<regType>.local."
    // If no name is given, fall back to the computer's DNS hostname as the service name.
    std::wstring instanceName;
    if (name != nullptr && name[0] != '\0')
    {
        auto r = dnssd::toWideString (name, instanceName);
        if (r.hasError()) return r;
    }
    else
    {
        auto r = getDnsHostname (instanceName);
        if (r.hasError()) return r;
    }

    std::wstring regTypeW;
    auto r = dnssd::toWideString (regType, regTypeW);
    if (r.hasError()) return r;

    // Accept both "_http._tcp" and "_http._tcp."
    while (!regTypeW.empty() && regTypeW.back() == L'.')
        regTypeW.pop_back();

    mInstanceName = instanceName + L"." + regTypeW + L".local.";
    mPort = port;

    return doRegister (txtRecord);
}

Result WindnsAdvertiser::updateTxtRecord (const TxtRecord& txtRecord)
{
    {
        std::lock_guard<std::mutex> lock (mLock);
        if (mState == State::Idle)
            return {};
    }

    // WinDNS has no in-place TXT update API; deregister and re-register.
    unregisterService();

    {
        std::lock_guard<std::mutex> lock (mLock);
        if (mState != State::Idle)
            return Result ("Failed to update TXT record: timed out waiting for the existing registration to be removed");
    }

    return doRegister (txtRecord);
}

void WindnsAdvertiser::unregisterService() noexcept
{
    std::unique_lock<std::mutex> lock (mLock);

    if (mState == State::Registering)
    {
        // Cancelling invokes the completion callback with ERROR_CANCELLED, which takes mLock.
        lock.unlock();
        DWORD res = DnsServiceRegisterCancel (&mCancel);
        if (res != ERROR_SUCCESS)
            DNSSD_LOG_DEBUG ("WindnsAdvertiser::unregisterService: DnsServiceRegisterCancel failed: "
                             << Result (static_cast<DNS_STATUS> (res)).description() << std::endl)
        lock.lock();

        // The registration might have completed before it could be cancelled, in which case the state will now be
        // Registered and the service is deregistered below.
        if (!waitWhileInState (lock, State::Registering))
        {
            DNSSD_LOG_DEBUG ("WindnsAdvertiser::unregisterService: timed out waiting for register callback" << std::endl)
            return;
        }
    }

    if (mState != State::Registered)
        return;

    mState = State::Deregistering;
    lock.unlock();
    DWORD status = DnsServiceDeRegister (&mReq, nullptr);
    lock.lock();

    if (status == DNS_REQUEST_PENDING)
    {
        if (!waitWhileInState (lock, State::Deregistering))
            DNSSD_LOG_DEBUG ("WindnsAdvertiser::unregisterService: timed out waiting for deregister callback" << std::endl)
    }
    else if (mState == State::Deregistering)
    {
        // Completed synchronously or failed, either way no callback will follow.
        if (status != ERROR_SUCCESS)
            DNSSD_LOG_DEBUG ("WindnsAdvertiser::unregisterService: DnsServiceDeRegister failed: "
                             << Result (static_cast<DNS_STATUS> (status)).description() << std::endl)
        mState = State::Idle;
    }
}

WindnsAdvertiser::~WindnsAdvertiser()
{
    unregisterService();

    if (mReq.pServiceInstance != nullptr)
    {
        DnsServiceFreeInstance (mReq.pServiceInstance);
        mReq.pServiceInstance = nullptr;
    }
}

Result WindnsAdvertiser::doRegister (const TxtRecord& txtRecord)
{
    std::wstring hostname;
    auto r = getDnsHostname (hostname);
    if (r.hasError()) return r;
    hostname += L".local";

    // Build TXT record key/value arrays.
    // wstrings must stay alive until DnsServiceConstructInstance returns.
    std::vector<std::wstring> keyStrings, valueStrings;
    keyStrings.reserve (txtRecord.size());
    valueStrings.reserve (txtRecord.size());
    for (const auto& kv : txtRecord)
    {
        std::wstring key, value;
        auto rKey = dnssd::toWideString (kv.first, key);
        if (rKey.hasError()) return rKey;
        auto rValue = dnssd::toWideString (kv.second, value);
        if (rValue.hasError()) return rValue;
        keyStrings.push_back (std::move (key));
        valueStrings.push_back (std::move (value));
    }

    std::vector<PCWSTR> keys, values;
    keys.reserve (keyStrings.size());
    values.reserve (valueStrings.size());
    for (size_t i = 0; i < keyStrings.size(); ++i)
    {
        keys.push_back (keyStrings[i].c_str());
        values.push_back (valueStrings[i].c_str());
    }

    PDNS_SERVICE_INSTANCE instance = DnsServiceConstructInstance (
        mInstanceName.c_str(),
        hostname.c_str(),
        nullptr, // IPv4: let the system determine the address per interface
        nullptr, // IPv6: let the system determine the address per interface
        mPort,
        0, // priority
        0, // weight
        static_cast<DWORD> (keyStrings.size()),
        keys.data(),
        values.data());

    if (instance == nullptr)
    {
        std::stringstream msg;
        msg << "DnsServiceConstructInstance failed. Error code: " << GetLastError();
        return dnssd::Result (msg.str());
    }

    if (mReq.pServiceInstance != nullptr)
        DnsServiceFreeInstance (mReq.pServiceInstance);

    mReq = {};
    mReq.Version = DNS_QUERY_REQUEST_VERSION1;
    mReq.InterfaceIndex = 0; // Register on all available network interfaces
    mReq.pServiceInstance = instance;
    mReq.pRegisterCompletionCallback = &WindnsAdvertiser::registerCompleteCallback;
    mReq.pQueryContext = this;
    mReq.hCredentials = nullptr;
    mReq.unicastEnabled = FALSE;

    mCancel = {};

    // Set the state before issuing the request, the callback might fire before DnsServiceRegister returns.
    {
        std::lock_guard<std::mutex> lock (mLock);
        mState = State::Registering;
    }

    DWORD status = DnsServiceRegister (&mReq, &mCancel);
    if (status != DNS_REQUEST_PENDING)
    {
        {
            std::lock_guard<std::mutex> lock (mLock);
            mState = State::Idle;
        }
        DnsServiceFreeInstance (instance);
        mReq.pServiceInstance = nullptr;
        return dnssd::Result (static_cast<DNS_STATUS> (status));
    }

    return {};
}

bool WindnsAdvertiser::waitWhileInState (std::unique_lock<std::mutex>& lock, State state)
{
    return mCv.wait_for (lock, kWindnsCallbackTimeout, [this, state]() { return mState != state; });
}

VOID WINAPI WindnsAdvertiser::registerCompleteCallback (DWORD Status, PVOID pQueryContext, PDNS_SERVICE_INSTANCE pInstance)
{
    DNSSD_LOG_DEBUG ("> registerCompleteCallback enter (status " << Status << ")" << std::endl)

    // The instance passed to the callback is owned by us.
    if (pInstance != nullptr)
        DnsServiceFreeInstance (pInstance);

    auto* self = static_cast<WindnsAdvertiser*> (pQueryContext);
    if (self == nullptr)
        return;

    Result error;
    AdvertiserErrorCallback errorCallback;

    {
        std::lock_guard<std::mutex> lock (self->mLock);

        if (self->mState == State::Registering)
        {
            if (Status == ERROR_SUCCESS)
            {
                self->mState = State::Registered;
            }
            else
            {
                self->mState = State::Idle;
                if (Status != ERROR_CANCELLED)
                    error = Result ("Registering DNS Service failed: " + Result (static_cast<DNS_STATUS> (Status)).description());
            }
        }
        else if (self->mState == State::Deregistering)
        {
            self->mState = State::Idle;
            if (Status != ERROR_SUCCESS)
                error = Result ("Deregistering DNS Service failed: " + Result (static_cast<DNS_STATUS> (Status)).description());
        }
        else
        {
            DNSSD_LOG_DEBUG ("- registerCompleteCallback: unexpected callback, ignoring" << std::endl)
        }

        if (error.hasError())
            errorCallback = self->onAdvertiserErrorCallback;

        self->mCv.notify_all();
    }

    // Don't touch self from here on: once mLock is released the owner may be destroyed.
    if (error.hasError())
    {
        DNSSD_LOG_DEBUG (error.description() << std::endl)
        if (errorCallback)
            errorCallback (error);
    }

    DNSSD_LOG_DEBUG ("< registerCompleteCallback exit" << std::endl)
}

} // namespace dnssd
