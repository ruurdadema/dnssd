# DNS Service Discovery for C++
This library provides an interface which allows to use dns-sd in an easy way.

## Prerequisites

* C++11 (or higher)
* CMake (3.15 or higher)

### MacOS
  
* XCode

### Windows

* Bonjour SDK (Apple)  
This library uses the Bonjour SDK and expects it to be in the default install location (ie C:\Program Files\Bonjour SDK).

### Linux - Not supported yet
Do you want to use this library on Linux (using Avahi)? Ping me a message.

## How to use

### Browsing

    #include <dnssd/Browser.h>

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
    
        void onBrowserErrorAsync (const dnssd::Result& error) override
        {
            std::cout << "Result: " << error.description() << std::endl;
        }
    };

    MyBrowser browser;

    // Start browsing
    if (auto error = browser.browseFor("_http._tcp"))
    {
        std::cout << "Result: " << error.description() << std::endl;
    };
    
### Advertising

    #include <dnssd/Advertiser.h>

    class MyAdvertiser : public dnssd::Advertiser
    {
    public:
        void onAdvertiserErrorAsync (const dnssd::Result& error) override
        {
            std::cout << "Result: " << error.description() << std::endl;
        }
    };

    MyAdvertiser advertiser;

    dnssd::TxtRecord txtRecord = {{"key1", "value1"}, {"key2", "value2"}};

    // Register service
    if (auto error = advertiser.registerService("_http._tcp", 80, txtRecord))
    {
        std::cout << "Result: " << error.description() << std::endl;
    }

## How to build

## As subdirectory

The easiest way of using this libary is to include it as subdirectory in your CMake project:

    add_subdirectory(./path/to/this/directory)
    
And to link your targets to the library:

    target_link_libraries(target PRIVATE dnssd-cpp)
    
## As separate build

If you don't use CMake for building then this way will produce a separate library:

    mkdir build 
    cd build
    cmake ..
    cmake --build .
    
After this you end up with two test command line utilities and a libray
