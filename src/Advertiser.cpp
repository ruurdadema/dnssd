#include <dnssd/Advertiser.h>
#include <dnssd/bonjour/BonjourAdvertiser.h>

#include <iostream>
#include <thread>

using namespace dnssd;

dnssd::Advertiser::Advertiser(const Listener& listener) : impl(new BonjourAdvertiser(listener))
{
}

Advertiser::~Advertiser()
{
    impl->unregisterService();
}

Error Advertiser::registerService(const std::string& serviceName, uint16_t port) noexcept
{
    return impl->registerService(serviceName, port);
}

Error Advertiser::registerService(
    const std::string &serviceName, uint16_t port,
    const TXTRecord &txtRecord) noexcept
{
    return impl->registerService(serviceName, port, txtRecord);
}

Error Advertiser::registerService(
    const std::string &serviceName, uint16_t port,
    const std::map<std::string, std::string>& keysValues) noexcept
{
    return impl->registerService(serviceName, port, keysValues);
}
