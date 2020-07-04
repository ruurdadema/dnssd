/*
  ==============================================================================

    DNSSDError.h
    Created: 21 Jun 2020 8:04:45pm
    Author:  Ruurd Adema

  ==============================================================================
*/

#pragma once

#include <string>
#include <exception>

#include <dnssd/bonjour/Bonjour.h>

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
