#include <dnssd/Browser.h>

#include <iostream>
#include <string>

int main (int argc, char* argv[])
{
    if (argc < 2)
    {
        std::cout << "Expected an argument which specifies the servicetype to browse for (example: _http._tcp)" << std::endl;
        return -1;
    }

    dnssd::Browser browser;

    browser.onServiceDiscoveredAsync = [] (const dnssd::ServiceDescription& service) {
        std::cout << "Service discovered: " << service.description() << std::endl;
    };

    browser.onServiceRemovedAsync = [] (const dnssd::ServiceDescription& service) {
        std::cout << "Service removed: " << service.description() << std::endl;
    };

    browser.onServiceResolvedAsync = [] (const dnssd::ServiceDescription& service, uint32_t interfaceIndex) {
        std::cout << "Service resolved: " << service.description() << std::endl;
    };

    browser.onAddressAddedAsync =
        [] (const dnssd::ServiceDescription& service, const std::string& address, uint32_t interfaceIndex) {
            std::cout << "Address added (" << address << "): " << service.description() << std::endl;
        };

    browser.onAddressRemovedAsync =
        [] (const dnssd::ServiceDescription& service, const std::string& address, uint32_t interfaceIndex) {
            std::cout << "Address removed (" << address << "): " << service.description() << std::endl;
        };

    browser.onBrowserErrorAsync = [] (const dnssd::Error& error) {
        std::cout << "Error: " << error.description() << std::endl;
    };

    if (auto error = browser.browseFor (argv[1]))
    {
        std::cout << "Error: " << error.description() << std::endl;
        return -1;
    };

    std::cout << "Press enter to exit..." << std::endl;

    std::string cmd;
    std::getline (std::cin, cmd);

    std::cout << "Exit" << std::endl;
}
