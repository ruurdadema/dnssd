//
// Created by Ruurd Adema on 04/07/2020.
//

#include <dnssd/bonjour/BonjourAdvertiser.h>
#include <dnssd/bonjour/BonjourTXTRecord.h>

#include <iostream>
#include <thread>

dnssd::BonjourAdvertiser::BonjourAdvertiser(const Listener& listener) :
    CommonAdvertiserInterface(listener)
{
}

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
        owner->callListener([error](const dnssd::CommonAdvertiserInterface::Listener& observer)
                            { observer.onAdvertiserErrorAsync(error); });
        owner->unregisterService();
        return;
    }
}

dnssd::Error dnssd::BonjourAdvertiser::registerService(const std::string& serviceName, uint16_t port) noexcept
{
    DNSServiceRef serviceRef = nullptr;

    if (auto error = Error(DNSServiceRegister(&serviceRef, 0, 0, nullptr, serviceName.c_str(), nullptr, nullptr,
                                   htons(port), 0, nullptr, registerServiceCallBack, this)))
    {
        return error;
    };

    mServiceRef = serviceRef;

    return Error(DNSServiceProcessResult(mServiceRef));
}

dnssd::Error dnssd::BonjourAdvertiser::registerService(
    const std::string &serviceName, uint16_t port,
    const TxtRecord& txtRecord) noexcept
{
    DNSServiceRef serviceRef = nullptr;
    auto record = BonjourTXTRecord(txtRecord);

    if (auto error = Error(DNSServiceRegister(&serviceRef, 0, 0, nullptr, serviceName.c_str(), nullptr, nullptr,
                                         htons(port), record.length(), record.bytesPtr(), registerServiceCallBack,
                                         this)))
    {
        return error;
    }

    mServiceRef = serviceRef;

    return Error(DNSServiceProcessResult(mServiceRef));
}

dnssd::Error dnssd::BonjourAdvertiser::registerService(
    const std::string &serviceName, uint16_t port,
    const std::map<std::string, std::string>& keysValues) noexcept
{
    BonjourTXTRecord txtRecord = BonjourTXTRecord(keysValues);

    DNSServiceRef serviceRef = nullptr;

    if (auto error = Error(DNSServiceRegister(
        &serviceRef,
        0,
        0,
        nullptr,
        serviceName.c_str(),
        nullptr, nullptr,
        htons(port),
        txtRecord.length(),
        txtRecord.bytesPtr(),
        registerServiceCallBack,
        this)))
    {
        return error;
    }

    mServiceRef = serviceRef;

    return Error(DNSServiceProcessResult(mServiceRef));
}

void dnssd::BonjourAdvertiser::unregisterService() noexcept
{
    if (mServiceRef) {
        DNSServiceRefDeallocate(mServiceRef);
        mServiceRef = nullptr;
    }
}
