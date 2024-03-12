#include <dnssd/bonjour/BonjourAdvertiser.h>
#include <dnssd/bonjour/BonjourTxtRecord.h>

#include <iostream>
#include <thread>

dnssd::Result dnssd::BonjourAdvertiser::registerService (
    const std::string& regType,
    const char* name,
    const char* domain,
    uint16_t port,
    const TxtRecord& txtRecord) noexcept
{
    DNSServiceRef serviceRef = nullptr;
    auto record = BonjourTxtRecord (txtRecord);

    auto result = Result (DNSServiceRegister (
        &serviceRef,
        0,
        0,
        name,
        regType.c_str(),
        domain,
        nullptr,
        htons (port),
        record.length(),
        record.bytesPtr(),
        registerServiceCallBack,
        this));

    if (result.hasError())
        return result;

    mServiceRef = serviceRef;

    return Result (DNSServiceProcessResult (mServiceRef.serviceRef()));
}

void dnssd::BonjourAdvertiser::unregisterService() noexcept
{
    mServiceRef = nullptr;
}

void dnssd::BonjourAdvertiser::registerServiceCallBack (
    DNSServiceRef serviceRef,
    DNSServiceFlags flags,
    DNSServiceErrorType errorCode,
    const char* serviceName,
    const char* regType,
    const char* replyDomain,
    void* context)
{
    (void)serviceRef;
    (void)flags;
    (void)serviceName;
    (void)regType;
    (void)replyDomain;

    Result const result (errorCode);

    if (result.hasError())
    {
        auto* owner = static_cast<BonjourAdvertiser*> (context);
        if (owner->onAdvertiserErrorCallback)
            owner->onAdvertiserErrorCallback (result);
        owner->unregisterService();
    }
}

dnssd::Result dnssd::BonjourAdvertiser::updateTxtRecord (const TxtRecord& txtRecord)
{
    auto const record = BonjourTxtRecord (txtRecord);

    // Second argument's nullptr tells us that we are updating the primary record.
    return Result (
        DNSServiceUpdateRecord (mServiceRef.serviceRef(), nullptr, 0, record.length(), record.bytesPtr(), 0));
}
