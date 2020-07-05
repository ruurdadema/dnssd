//
// Created by Ruurd Adema on 28/06/2020.
//

#include <dnssd/Browser.h>
#include <dnssd/common/Log.h>
#include <dnssd/bonjour/BonjourBrowser.h>

#include <string>
#include <thread>
#include <memory>

dnssd::Browser::Browser(const Listener& listener):
    impl(new BonjourBrowser(listener))
{
}

dnssd::Error dnssd::Browser::browseFor(const std::string& service)
{
    return impl->browseFor(service);
}
