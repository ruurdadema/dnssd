#include <dnssd/Advertiser.h>

#include <string>
#include <iostream>

class AdvertiserListener : public dnssd::CommonAdvertiserInterface::Listener
{
public:

    void onAdvertiserErrorAsync(dnssd::Error error) const noexcept override
    {
        std::cout << "Error: " << error.description() << std::endl;
    }
};

int main(int argc, char *argv[])
{
    AdvertiserListener listener;
    dnssd::Advertiser advertiser(listener);

    if (auto error = advertiser.registerService("_testservice._tcp", 512, {{"key1", "value1"}, {"key2", "value2"}}))
    {
        std::cout << "Error: " << error.description() << std::endl;
        return -1;
    }

    std::cout << "Press enter to continue..." << std::endl;

    std::string cmd;
    std::getline(std::cin, cmd);

    std::cout << "Exit" << std::endl;
}