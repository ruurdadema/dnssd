#include <dnssd/Advertiser.h>

#include <string>
#include <iostream>
#include <vector>

int main(int argc, char* argv[])
{
    std::vector<std::string> args;
    for (int i = 1; i < argc; i++) { args.emplace_back(argv[i]); }

    // Do we have at least the service type and port number?
    if (args.size() < 2)
    {
        std::cout << "Error: expected at least an argument which specifies the service type and an argument which specifies the port number" << std::endl;
        return -1;
    }

    // Parse port number
    int portNumber = 0;
    try
    {
        portNumber = std::stoi(args[1]);
    }
    catch (const std::exception& e)
    {
        std::cout << "Invalid port number: " << e.what() << std::endl;
        return -1;
    }

    // Parse remaining arguments as TxtRecord
    dnssd::TxtRecord txtRecord;
    for (auto it = args.begin() + 2; it != args.end(); ++it)
    {
        size_t pos = it->find("=");
        if (pos != std::string::npos)
        {
            txtRecord.insert({it->substr(0, pos), it->substr(pos+1)});
        }
        else
        {
            txtRecord.insert({*it, ""});
        }
    }

    dnssd::Advertiser advertiser;
    advertiser.onAdvertiserErrorAsync = [](const dnssd::Error& error)
    {
        std::cout << "Error: " << error.description() << std::endl;
    };

    if (auto error = advertiser.registerService(args[0], portNumber, txtRecord, "001122334455@SomeName"))
    {
        std::cout << "Error: " << error.description() << std::endl;
        return -1;
    }

    std::cout << "Press enter to continue..." << std::endl;

    std::string cmd;
    std::getline(std::cin, cmd);

    std::cout << "Exit" << std::endl;
}
