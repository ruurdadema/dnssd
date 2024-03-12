#include <dnssd/Browser.h>

#include <iostream>
#include <string>

int main (int argc, char* argv[])
{
    if (argc < 2)
    {
        std::cout << "Expected an argument which specifies the servicetype to browse for (example: _http._tcp)"
                  << std::endl;
        return -1;
    }

    dnssd::Browser browser;

    browser.onServiceDiscovered ([] (const dnssd::ServiceDescription& serviceDescription) {
        std::cout << "Service discovered: " << serviceDescription.description() << std::endl;
    });

    browser.onServiceRemoved ([] (const dnssd::ServiceDescription& serviceDescription) {
        std::cout << "Service removed: " << serviceDescription.description() << std::endl;
    });

    browser.onServiceResolved ([] (const dnssd::ServiceDescription& serviceDescription, uint32_t interfaceIndex) {
        std::cout << "Service resolved: " << serviceDescription.description() << std::endl;
    });

    browser.onAddressAdded (
        [] (const dnssd::ServiceDescription& serviceDescription, const std::string& address, uint32_t interfaceIndex) {
            std::cout << "Address added (" << address << "): " << serviceDescription.description() << std::endl;
        });

    browser.onAddressRemoved (
        [] (const dnssd::ServiceDescription& serviceDescription, const std::string& address, uint32_t interfaceIndex) {
            std::cout << "Address removed (" << address << "): " << serviceDescription.description() << std::endl;
        });

    browser.onBrowseError ([] (const dnssd::Result& error) {
        std::cout << "Error: " << error.description() << std::endl;
    });

    auto const result = browser.browseFor (argv[1]);
    if (result.hasError())
    {
        std::cout << "Error: " << result.description() << std::endl;
        return -1;
    };

    std::cout << "Press enter to exit..." << std::endl;

    std::string cmd;
    std::getline (std::cin, cmd);

    std::cout << "Exit" << std::endl;
}
