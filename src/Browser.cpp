//
// Created by Ruurd Adema on 28/06/2020.
//

#include <dnssd/Browser.h>
#include <dnssd/common/Debug.h>
#include <dnssd/bonjour/BonjourBrowserImpl.h>

#include <string>
#include <thread>
#include <memory>

dnssd::Browser::Browser(const Listener& listener):
    impl(new BonjourBrowserImpl(listener))
{
}

dnssd::Error dnssd::Browser::browseFor(const std::string& service)
{
    return impl->browseFor(service);
}
