cmake_minimum_required(VERSION 3.15)

project(dns-sd-cpp VERSION 0.0.1)

set(CMAKE_CXX_STANDARD 11)

add_library(${PROJECT_NAME} STATIC
        include/dnssd/Browser.h
        include/dnssd/internal/ScopedDNSServiceRef.h
        include/dnssd/internal/Service.h
        include/dnssd/internal/SharedConnection.h
        src/Advertiser.cpp
        src/Browser.cpp
        src/ServiceDescription.cpp
        src/internal/Debug.cpp
        src/internal/Error.cpp
        src/internal/Error.cpp/
        src/internal/ScopedDNSServiceRef.cpp
        src/internal/Service.cpp
        src/internal/SharedConnection.cpp
        src/internal/TXTRecord.cpp include/dnssd/bonjour/Bonjour.h)

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

add_executable(dns-sd-advertiser test/dns-sd-advertiser.cpp)
target_link_libraries(dns-sd-advertiser ${PROJECT_NAME})

add_executable(dns-sd-browser test/dns-sd-browser.cpp)
target_link_libraries(dns-sd-browser ${PROJECT_NAME})
