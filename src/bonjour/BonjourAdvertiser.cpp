#include <dnssd/bonjour/BonjourAdvertiser.h>
#include <dnssd/bonjour/BonjourTxtRecord.h>

#include <iostream>
#include <thread>

static void DNSSD_API registerServiceCallBack (
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

    dnssd::Result error (errorCode);

    if (error)
    {
        auto* owner = static_cast<dnssd::BonjourAdvertiser*> (context);
        owner->onAdvertiserErrorAsync (error);
        owner->unregisterService();
        return;
    }
}

dnssd::Result dnssd::BonjourAdvertiser::registerService (
    const std::string& regType,
    const char* name,
    const char* domain,
    uint16_t port,
    const TxtRecord& txtRecord) noexcept
{
    DNSServiceRef serviceRef = nullptr;
    auto record = BonjourTxtRecord (txtRecord);

    if (auto error = Result (DNSServiceRegister (
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
            this)))
    {
        return error;
    }

    mServiceRef = serviceRef;

    return Result (DNSServiceProcessResult (mServiceRef.serviceRef()));
}

void dnssd::BonjourAdvertiser::unregisterService() noexcept
{
    mServiceRef = nullptr;
}

dnssd::Result dnssd::BonjourAdvertiser::updateTxtRecord (const dnssd::TxtRecord& txtRecord)
{
    auto record = BonjourTxtRecord (txtRecord);

    // Second argument's nullptr tells us that we are updating the primary record.
    return Result (
        DNSServiceUpdateRecord (mServiceRef.serviceRef(), nullptr, 0, record.length(), record.bytesPtr(), 0));
}
