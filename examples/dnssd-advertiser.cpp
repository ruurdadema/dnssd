#include <dnssd/Advertiser.h>

#include <iostream>
#include <string>
#include <vector>

bool parseTxtRecord (dnssd::TxtRecord& txtRecord, const std::string& stringValue)
{
    if (stringValue.empty())
        return false;

    size_t pos = stringValue.find ('=');

    if (pos != std::string::npos)
        txtRecord[stringValue.substr (0, pos)] = stringValue.substr (pos + 1);
    else
        txtRecord[stringValue.substr (0, pos)] = "";

    return true;
}

int main (int const argc, char* argv[])
{
    std::vector<std::string> args;
    for (int i = 1; i < argc; i++)
    {
        args.emplace_back (argv[i]);
    }

    // Do we have at least the service type and port number?
    if (args.size() < 2)
    {
        std::cout << "Error: expected at least an argument which specifies the service type and an argument which "
                     "specifies the port number (example: _test._tcp 1234 key1=value1 key2=value2)"
                  << std::endl;
        return -1;
    }

    // Parse port number
    int portNumber = 0;
    try
    {
        portNumber = std::stoi (args[1]);
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
        parseTxtRecord (txtRecord, *it);
    }

    dnssd::Advertiser advertiser;

    advertiser.onAdvertiserErrorAsync ([] (const dnssd::Result& error) {
        std::cout << "Error: " << error.description() << std::endl;
    });

    auto result = advertiser.registerService (args[0], "001122334455@SomeName", nullptr, portNumber, txtRecord);
    if (result.hasError())
    {
        std::cout << "Error: " << result.description() << std::endl;
        return -1;
    }

    std::cout << "Enter key=value to update the TXT record, or q to exit..." << std::endl;

    std::string cmd;
    while (true)
    {
        std::getline (std::cin, cmd);
        if (cmd == "q" || cmd == "Q")
        {
            break;
        }

        if (parseTxtRecord (txtRecord, cmd))
        {
            std::cout << "Updated txt record: " << std::endl;
            for (auto& pair : txtRecord)
            {
                std::cout << pair.first << "=" << pair.second << std::endl;
            }

            advertiser.updateTxtRecord (txtRecord);
        }
    }

    std::cout << "Exit" << std::endl;

    return 0;
}
