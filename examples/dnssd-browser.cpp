#include <dnssd/Browser.h>

#include <iostream>
#include <string>

class MyBrowser : public dnssd::Browser
{
public:
    void onServiceDiscoveredAsync (const dnssd::ServiceDescription& serviceDescription) override
    {
        std::cout << "Service discovered: " << serviceDescription.description() << std::endl;
    }

    void onServiceRemovedAsync (const dnssd::ServiceDescription& serviceDescription) override
    {
        std::cout << "Service removed: " << serviceDescription.description() << std::endl;
    }

    void onServiceResolvedAsync (const dnssd::ServiceDescription& serviceDescription, uint32_t interfaceIndex) override
    {
        std::cout << "Service resolved: " << serviceDescription.description() << std::endl;
    }

    void onAddressAddedAsync (
        const dnssd::ServiceDescription& serviceDescription,
        const std::string& address,
        uint32_t interfaceIndex) override
    {
        std::cout << "Address added (" << address << "): " << serviceDescription.description() << std::endl;
    }

    void onAddressRemovedAsync (
        const dnssd::ServiceDescription& serviceDescription,
        const std::string& address,
        uint32_t interfaceIndex) override
    {
        std::cout << "Address removed (" << address << "): " << serviceDescription.description() << std::endl;
    }

    void onBrowserErrorAsync (const dnssd::Result& error) override
    {
        std::cout << "Error: " << error.description() << std::endl;
    }
};

int main (int argc, char* argv[])
{
    if (argc < 2)
    {
        std::cout << "Expected an argument which specifies the servicetype to browse for (example: _http._tcp)"
                  << std::endl;
        return -1;
    }

    MyBrowser browser;

    auto result = browser.browseFor (argv[1]);
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
