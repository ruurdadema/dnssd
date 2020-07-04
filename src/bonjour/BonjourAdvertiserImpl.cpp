//
// Created by Ruurd Adema on 04/07/2020.
//

#include <dnssd/bonjour/BonjourAdvertiserImpl.h>

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
        auto* owner = static_cast<dnssd::BonjourAdvertiserImpl*>(context);
        owner->callObserver([error](const dnssd::Advertiser::Observer& observer){ observer.onAdvertiserError(error); });
        owner->unregisterService();
        return;
    }
}

dnssd::Error dnssd::BonjourAdvertiserImpl::registerService(const std::string& serviceName, uint16_t port) noexcept
{
    Error error(DNSServiceRegister(&mServiceRef, 0, 0, nullptr, serviceName.c_str(), nullptr, nullptr,
                                   htons(port), 0, nullptr, registerServiceCallBack, this));

    if (error) { return error; }

    return Error(DNSServiceProcessResult(mServiceRef));
}

dnssd::Error dnssd::BonjourAdvertiserImpl::registerService(
    const std::string &serviceName, uint16_t port,
    const TXTRecord &txtRecord) noexcept
{

    std::cout << "Register service (thread id: " << std::this_thread::get_id() << ")" << std::endl;

    const Error error(DNSServiceRegister(&mServiceRef, 0, 0, nullptr, serviceName.c_str(), nullptr, nullptr,
                                         htons(port), txtRecord.length(), txtRecord.bytesPtr(), registerServiceCallBack,
                                         this));
    if (error) { return error; }

    return Error(DNSServiceProcessResult(mServiceRef));
}

dnssd::Error dnssd::BonjourAdvertiserImpl::registerService(
    const std::string &serviceName, uint16_t port,
    const std::map<std::string, std::string>& keysValues) noexcept
{
    TXTRecord txtRecord = TXTRecord();

    for (auto& keyValue : keysValues) {
        txtRecord.setValue(keyValue.first, keyValue.second);
    }

    const Error error(DNSServiceRegister(
        &mServiceRef,
        0,
        0,
        nullptr,
        serviceName.c_str(),
        nullptr, nullptr,
        htons(port),
        txtRecord.length(),
        txtRecord.bytesPtr(),
        registerServiceCallBack,
        this));

    if (error) { return error; }

    return Error(DNSServiceProcessResult(mServiceRef));
}


void dnssd::BonjourAdvertiserImpl::unregisterService() noexcept
{
    if (mServiceRef) {
        DNSServiceRefDeallocate(mServiceRef);
        mServiceRef = nullptr;
    }
}

void dnssd::BonjourAdvertiserImpl::callObserver(std::function<void(Advertiser::Observer&)> callback) noexcept
{
    if (mObserver != nullptr) { callback(*mObserver); }
}
