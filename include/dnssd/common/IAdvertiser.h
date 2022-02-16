#pragma once

#include "../common/TxtRecord.h"
#include "Result.h"
#include "Util.h"

#include <functional>

namespace dnssd
{

/**
 * Interface class which represents a dnssd advertiser object, which is able to present itself onto the network.
 */
class IAdvertiser
{
public:
    explicit IAdvertiser() = default;
    virtual ~IAdvertiser() = default;

    /**
     * Registers a service with given arguments.
     * @param serviceName The name of the service.
     * @param port The port of the service.
     * @param name The name of the service, which is optional. If no name is given then the service will be named based
     * on the computer name which in most cases makes sense to do.
     * @param txtRecord A TXT record to add to the service, consisting of a couple of keys and values.
     * @return An Result indicating success or failure.
     */
    virtual Result registerService (
        const std::string& serviceName,
        uint16_t port,
        const char* name,
        const TxtRecord& txtRecord) noexcept = 0;

    /**
     * Updates the TXT record of this service. The given TXT record will replace the previous one.
     * @param txtRecord The new TXT record.
     * @return An Result indicating success or failure.
     */
    virtual Result updateTxtRecord (const TxtRecord& txtRecord) = 0;

    /**
     * Unregisters this service from the mDnsResponder, after which the service will no longer be found on the network.
     */
    virtual void unregisterService() noexcept = 0;

    /**
     * Called when there was an error.
     * Note: this call will be made from a background thread and wil not be synchronised.
     * @param error The error which occured.
     */
    virtual void onAdvertiserErrorAsync (const Result& error) { ignore (error); }
};

} // namespace dnssd
