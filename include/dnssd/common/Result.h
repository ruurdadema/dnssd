#pragma once

#include <exception>
#include <string>

#include <dnssd/bonjour/Bonjour.h>

namespace dnssd
{

/**
 * Class for indicating success or failure. It can hold a DNSServiceErrorType and an error message where either will
 * make the Result indicate an error.
 */
class Result
{
public:
    Result() = default;

    /**
     * Construct this Result with given DNSServiceErrorType.
     * @param error The DNSServiceErrorType to store.
     */
    explicit Result (DNSServiceErrorType error) noexcept;

    /**
     * Constructs this Result with an error message.
     * @param errorMsg The error message to store.
     */
    explicit Result (const std::string& errorMsg) noexcept;

    /**
     * @return Returns true if this result holds an error.
     */
    bool hasError() const;

    /**
     * @return Returns true if this result doesn't hold an error.
     */
    bool isOk() const;

    /**
     * @return Returns a description of this Result, which will either return the error message or a description of the stored
     * DNSServiceErrorType.
     */
    std::string description() const noexcept;

private:
    DNSServiceErrorType mError = kDNSServiceErr_NoError;
    std::string mErrorMsg;

    static const char* DNSServiceErrorDescription (DNSServiceErrorType error) noexcept;
};

} // namespace dnssd
