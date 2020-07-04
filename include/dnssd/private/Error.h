/*
  ==============================================================================

    DNSSDError.h
    Created: 21 Jun 2020 8:04:45pm
    Author:  Ruurd Adema

  ==============================================================================
*/

#pragma once

#ifdef _WINDOWS_
#error "winsock2.h has to be included before windows.h please include this headerfile before that including windows.h"
#endif

#ifdef _WIN32
#define _WINSOCKAPI_ // Prevents inclusion of winsock.h in windows.h
#include <winsock2.h>
#include <Ws2tcpip.h>
#else
#include <arpa/inet.h>
#endif

#include <dns_sd.h>

#include <dns_sd.h>
#include <string>
#include <exception>

namespace dnssd {

class Error
{
public:
    Error() = default;
    Error(DNSServiceErrorType error) noexcept;
    Error(const std::string& errorMsg) noexcept;

    explicit operator bool() const { return mError != kDNSServiceErr_NoError || !mErrorMsg.empty(); }

    std::string description() const noexcept;
private:
    DNSServiceErrorType mError = kDNSServiceErr_NoError;
    std::string mErrorMsg;

    static const char* DNSServiceErrorDescription(DNSServiceErrorType error) noexcept;
};

class Exception: public std::exception
{
public:
    Exception(std::string  errorMsg);
    Exception(const Error& error);

    const char* what() const throw ();

private:
    std::string mErrorMsg;
};

} // namespace dnssd
