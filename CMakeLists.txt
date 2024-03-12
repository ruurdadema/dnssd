cmake_minimum_required(VERSION 3.15)

project(dnssd-cpp VERSION 0.0.1)

option(DNSSD_NO_EXAMPLES "Disable examples" OFF)

set(CMAKE_CXX_STANDARD 11)

add_library(dnssd-cpp STATIC
        include/dnssd/Browser.h
        include/dnssd/bonjour/Bonjour.h
        include/dnssd/bonjour/BonjourAdvertiser.h
        include/dnssd/bonjour/BonjourBrowser.h
        include/dnssd/bonjour/ScopedDnsServiceRef.h
        include/dnssd/bonjour/Service.h
        include/dnssd/bonjour/SharedConnection.h
        include/dnssd/common/IAdvertiser.h
        include/dnssd/common/IBrowser.h
        include/dnssd/common/TxtRecord.h
        include/dnssd/common/Util.h
        src/ServiceDescription.cpp
        src/bonjour/Bonjour.cpp
        src/bonjour/BonjourAdvertiser.cpp
        src/bonjour/BonjourBrowser.cpp
        src/bonjour/BonjourTxtRecord.cpp
        src/bonjour/ScopedDnsServiceRef.cpp
        src/bonjour/Service.cpp
        src/bonjour/SharedConnection.cpp
        src/common/Result.cpp
        src/common/Result.cpp
        src/common/Log.cpp
)

target_include_directories(dnssd-cpp PUBLIC include)

if (WIN32)
    set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} /NODEFAULTLIB:libcmt")

    target_include_directories(dnssd-cpp PUBLIC "C:/Program Files/Bonjour SDK/Include")

    # Link dnssd.lib depending on architecture
    if (MSVC_CXX_ARCHITECTURE_ID MATCHES "64")
        target_link_libraries(dnssd-cpp PUBLIC "C:/Program Files/Bonjour SDK/Lib/x64/dnssd.lib")
    elseif (MSVC_CXX_ARCHITECTURE_ID MATCHES "86")
        target_link_libraries(dnssd-cpp PUBLIC "C:/Program Files/Bonjour SDK/Lib/Win32/dnssd.lib")
    else ()
        message(FATAL_ERROR "Unknown MSVC architecture or non CXX language [${MSVC_CXX_ARCHITECTURE_ID}]")
    endif ()

    target_link_libraries(dnssd-cpp PUBLIC ws2_32)
endif ()

if (NOT DNSSD_NO_EXAMPLES)
    add_executable(dnssd-advertiser examples/dnssd-advertiser.cpp)
    target_link_libraries(dnssd-advertiser dnssd-cpp)

    add_executable(dnssd-browser examples/dnssd-browser.cpp)
    target_link_libraries(dnssd-browser dnssd-cpp)
endif ()
