#pragma once

#include <stdint.h>
#include <string>
#include <map>
#include <functional>

#include "dnssd/internal/Error.h"
#include "dnssd/bonjour/TXTRecord.h"

namespace dnssd {

class Advertiser
{
public:
    class Impl
    {
    public:
        virtual ~Impl() = default;
        virtual Error registerService(const std::string& serviceName, uint16_t port) noexcept = 0;
        virtual Error registerService(const std::string& serviceName, uint16_t port, const TXTRecord& txtRecord) noexcept = 0;
        virtual Error registerService(const std::string& serviceName, uint16_t port, const std::map<std::string, std::string>& keysValues) noexcept = 0;
        virtual void unregisterService() noexcept = 0;
    };

    class Listener
    {
    public:
        virtual ~Listener() = default;
        virtual void onAdvertiserErrorAsync(Error error) const noexcept = 0;
    };

    explicit Advertiser(const Listener& listener);
    ~Advertiser();

    Error registerService(const std::string& serviceName, uint16_t port) noexcept;
    Error registerService(const std::string& serviceName, uint16_t port, const TXTRecord& txtRecord) noexcept;
    Error registerService(const std::string& serviceName, uint16_t port, const std::map<std::string, std::string>& keysValues) noexcept;

private:
    std::unique_ptr<Impl> impl;
};

} // namespace dnssd
