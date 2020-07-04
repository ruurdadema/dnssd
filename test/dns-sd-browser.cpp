#include <dnssd/Browser.h>

#include <string>
#include <iostream>

class BrowserListener : public dnssd::Browser::Listener
{
public:
    void onServiceDiscoveredAsync(const dnssd::ServiceDescription& service) const noexcept override
    {
        std::cout << "Service discovered: " << service.description() << std::endl;
    }

    void onServiceRemovedAsync(const dnssd::ServiceDescription& service) const noexcept override
    {
        std::cout << "Service removed: " << service.description() << std::endl;
    }

    void onServiceResolvedAsync(const dnssd::ServiceDescription& service, uint32_t interfaceIndex) const noexcept override
    {
        std::cout << "Service resolved: " << service.description() << std::endl;
    }

    void onAddressAddedAsync(const dnssd::ServiceDescription& service, const std::string& address, uint32_t interfaceIndex) const noexcept override
    {
        std::cout << "Address added (" << address << "): " << service.description() << std::endl;
    }

    void onAddressRemovedAsync(const dnssd::ServiceDescription& service, const std::string& address, uint32_t interfaceIndex) const noexcept override
    {
        std::cout << "Address removed (" << address << "): " << service.description() << std::endl;
    }

    void onBrowserErrorAsync(dnssd::Error error) const noexcept override
    {
        std::cout << "Error: " << error.description() << std::endl;
    }
};

int main(int argc, char *argv[])
{
    BrowserListener listener;
    dnssd::Browser browser(listener);

    if (auto error = browser.browseFor("_http._tcp."))
    {
        std::cout << "Error: " << error.description() << std::endl;
        return -1;
    }

    std::cout << "Press enter to continue..." << std::endl;

    std::string cmd;
    std::getline(std::cin, cmd);

    std::cout << "Exit" << std::endl;
}