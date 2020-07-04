#pragma once

#include <stdint.h>
#include <string>
#include <map>
#include <functional>

#include "dnssd/internal/Error.h"
#include "dnssd/internal/TXTRecord.h"

namespace dnssd {

class Advertiser
{
public:
    class Observer
    {
    public:
        virtual ~Observer() = default;
        virtual void onAdvertiserError(Error error) const noexcept = 0;
    };

    Advertiser() = default;
    ~Advertiser();
    
    Error registerService(const std::string& serviceName, uint16_t port) noexcept;
    Error registerService(const std::string& serviceName, uint16_t port, const TXTRecord& txtRecord) noexcept;
    Error registerService(const std::string& serviceName, uint16_t port, const std::map<std::string, std::string>& keysValues) noexcept;

    void unregisterService() noexcept;
    void observer(Observer* observer) noexcept { mObserver = observer; }
    void callObserver(std::function<void(Observer&)>) noexcept;

private:
    DNSServiceRef mServiceRef = nullptr;
    Observer* mObserver = nullptr;
};

} // namespace dnssd
