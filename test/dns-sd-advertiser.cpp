#include <dnssd/Advertiser.h>

#include <string>
#include <iostream>

int main(int argc, char *argv[])
{
    dnssd::Advertiser advertiser;

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