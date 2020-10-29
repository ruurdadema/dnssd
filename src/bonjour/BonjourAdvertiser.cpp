#include <dnssd/bonjour/BonjourAdvertiser.h>
#include <dnssd/bonjour/BonjourTxtRecord.h>

#include <iostream>
#include <thread>

static void DNSSD_API registerServiceCallBack(DNSServiceRef serviceRef, DNSServiceFlags flags,
                                              DNSServiceErrorType errorCode, const char* serviceName,
                                              const char* regType, const char* replyDomain, void* context)
{
    (void) serviceRef;
    (void) flags;
    (void) serviceName;
    (void) regType;
    (void) replyDomain;

    dnssd::Error error(errorCode);

    if (error)
    {
        auto* owner = static_cast<dnssd::BonjourAdvertiser*>(context);
        if (owner->onAdvertiserErrorAsync) { owner->onAdvertiserErrorAsync(error); }
        owner->unregisterService();
        return;
    }
}

dnssd::Error dnssd::BonjourAdvertiser::registerService(const std::string& serviceName, uint16_t port, const char* name) noexcept
{
    DNSServiceRef serviceRef = nullptr;

    if (auto error = Error(DNSServiceRegister(
        &serviceRef,
        0,
        0,
        nullptr,
        serviceName.c_str(),
        nullptr,
        nullptr,
        htons(port),
        0,
        nullptr,
        registerServiceCallBack,
        this)))
    {
        return error;
    };

    mServiceRef = serviceRef;

    return Error(DNSServiceProcessResult(mServiceRef.serviceRef()));
}

dnssd::Error dnssd::BonjourAdvertiser::registerService(
    const std::string &serviceName, uint16_t port,
    const TxtRecord& txtRecord,
    const char* name) noexcept
{
    DNSServiceRef serviceRef = nullptr;
    auto record = BonjourTxtRecord(txtRecord);

    if (auto error = Error(DNSServiceRegister(
        &serviceRef,
        0,
        0,
        "001122334455@SomeName",
        serviceName.c_str(),
        nullptr,
        nullptr,
        htons(port),
        record.length(),
        record.bytesPtr(),
        registerServiceCallBack,
        this)))
    {
        return error;
    }

    mServiceRef = serviceRef;

    return Error(DNSServiceProcessResult(mServiceRef.serviceRef()));
}

void dnssd::BonjourAdvertiser::unregisterService() noexcept
{
    mServiceRef = nullptr;
}
