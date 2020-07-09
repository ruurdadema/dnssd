cmake_minimum_required(VERSION 3.15)

project(dnssd-cpp VERSION 0.0.1)

set(CMAKE_CXX_STANDARD 11)

add_library(${PROJECT_NAME} STATIC
        include/dnssd/Browser.h
        include/dnssd/bonjour/Bonjour.h
        include/dnssd/bonjour/BonjourAdvertiser.h
        include/dnssd/bonjour/BonjourBrowser.h
        include/dnssd/bonjour/ScopedDnsServiceRef.h
        include/dnssd/bonjour/Service.h
        include/dnssd/bonjour/SharedConnection.h
        include/dnssd/common/CommonAdvertiserInterface.h
        include/dnssd/common/CommonBrowserInterface.h
        include/dnssd/common/TxtRecord.h
        src/ServiceDescription.cpp
        src/bonjour/BonjourAdvertiser.cpp
        src/bonjour/BonjourBrowser.cpp
        src/bonjour/BonjourTxtRecord.cpp
        src/bonjour/ScopedDnsServiceRef.cpp
        src/bonjour/Service.cpp
        src/bonjour/SharedConnection.cpp
        src/common/Error.cpp
        src/common/Error.cpp/
        src/common/Log.cpp
        )

target_include_directories(${PROJECT_NAME} PUBLIC ${PROJECT_SOURCE_DIR}/include)

if (WIN32)
    set(CMAKE_EXE_LINKER_FLAGS  "${CMAKE_EXE_LINKER_FLAGS} /NODEFAULTLIB:libcmt")

    target_include_directories(${PROJECT_NAME} PUBLIC "C:/Program Files/Bonjour SDK/Include")

    # Link dnssd.lib depending on architecture
    if (MSVC_CXX_ARCHITECTURE_ID MATCHES "64")
        target_link_libraries(${PROJECT_NAME} PUBLIC "C:/Program Files/Bonjour SDK/Lib/x64/dnssd.lib")
    elseif (MSVC_CXX_ARCHITECTURE_ID MATCHES "86")
        target_link_libraries(${PROJECT_NAME} PUBLIC "C:/Program Files/Bonjour SDK/Lib/Win32/dnssd.lib")
    else ()
        message(FATAL_ERROR "Unknown MSVC architecture or non CXX language [${MSVC_CXX_ARCHITECTURE_ID}]")
    endif()
endif()

add_executable(dnssd-advertiser test/dns-sd-advertiser.cpp)
target_link_libraries(dnssd-advertiser ${PROJECT_NAME})

add_executable(dnssd-browser test/dns-sd-browser.cpp)
target_link_libraries(dnssd-browser ${PROJECT_NAME})
