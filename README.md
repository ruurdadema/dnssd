# DNS Service Discovery for C++
This library provides an interface which allows to use dns-sd in an easy way.

_This is a work In Progress! Don't expect this library to be production ready._

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

    dnssd::Browser browser;
    
    browser.onServiceDiscoveredAsync = [](const dnssd::ServiceDescription& service)
    {
        std::cout << "Service discovered: " << service.description() << std::endl;
    };

    browser.onServiceRemovedAsync = [](const dnssd::ServiceDescription& service)
    {
        std::cout << "Service removed: " << service.description() << std::endl;
    };

    // Catch errors
    browser.onBrowserErrorAsync = [](dnssd::Error error)
    {
        std::cout << "Error: " << error.description() << std::endl;
    };

    // Start browsing
    if (auto error = browser.browseFor("_http._tcp"))
    {
        std::cout << "Error: " << error.description() << std::endl;
    };
    
### Advertising

    #include <dnssd/Advertiser.h>

    dnssd::Advertiser advertiser;

    // Catch errors
    advertiser.onAdvertiserErrorAsync = [](const dnssd::Error& error)
    {
        std::cout << "Error: " << error.description() << std::endl;
    };

    dnssd::TxtRecord txtRecord = {{"key1", "value1"}, {"key2", "value2"}};

    // Register service
    if (auto error = advertiser.registerService("_http._tcp", 80, txtRecord))
    {
        std::cout << "Error: " << error.description() << std::endl;
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
