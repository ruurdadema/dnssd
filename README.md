# DNS Service Discovery for C++
This library provides an interface which allows to work with dns-sd in an easy way.

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

## How to build

## As subdirectory

The easiest way of using this libary is to include this CMake project as subdirectory:

    add_subdirectory(./path/to/this/directory)
    
And to link your targets to the library:

    target_link_libraries(target PRIVATE dnssd-cpp)
    
## As separate build

    mkdir build 
    cd build
    cmake ..
    cmake --build .
    
After this you end up with two test command line utilities and a libray
