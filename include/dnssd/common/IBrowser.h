#pragma once

#include "../ServiceDescription.h"
#include "Result.h"
#include "Util.h"

#include <functional>

namespace dnssd
{

/**
 * Interface class which represents a Bonjour browser.
 */
class IBrowser
{
public:
    IBrowser() = default;
    virtual ~IBrowser() = default;

    /**
     * Starts browsing for a service
     * @param serviceType The service type (ie. _http._tcp.).
     * @return Returns an Result indicating success or failure.
     */
    virtual Result browseFor (const std::string& serviceType) = 0;

    /**
     * Called when a service was discovered.
     * Note: this call will be made from a background thread and wil not be synchronised.
     * @param serviceDescription Reference to the ServiceDescription of this service.
     */
    virtual void onServiceDiscoveredAsync (const ServiceDescription& serviceDescription)
    {
        ignore (serviceDescription);
    }

    /**
     * Called when a service was removed.
     * Note: this call will be made from a background thread and wil not be synchronised.
     * @param serviceDescription The ServiceDescription of the removed service, which will also get removed after this
     * call.
     */
    virtual void onServiceRemovedAsync (const ServiceDescription& serviceDescription) { ignore (serviceDescription); }

    /**
     * Called when a service was resolved (ie. address information was resolved).
     * Note: this call will be made from a background thread and wil not be synchronised.
     * @param serviceDescription The ServiceDescription of the resolved service.
     * @param interfaceIndex The index of the interface the service was resolved on.
     */
    virtual void onServiceResolvedAsync (const ServiceDescription& serviceDescription, uint32_t interfaceIndex)
    {
        ignore (serviceDescription, interfaceIndex);
    }

    /**
     * Called when an address became available (ie. service became reachable on this address).
     * Note: this call will be made from a background thread and wil not be synchronised.
     * @param serviceDescription The ServiceDescription of the resolved service.
     * @param address The added address.
     * @param interfaceIndex The index of the interface the address was added to.
     */
    virtual void onAddressAddedAsync (
        const ServiceDescription& serviceDescription,
        const std::string& address,
        uint32_t interfaceIndex)
    {
        ignore (serviceDescription, address, interfaceIndex);
    }

    /**
     * Called when an address became unavailable (ie. service no longer reachable on this address).
     * Note: this call will be made from a background thread and wil not be synchronised.
     * @param serviceDescription The ServiceDescription of the resolved service.
     * @param address The removed address.
     * @param interfaceIndex The index of the interface where the address became unavailable.
     */
    virtual void onAddressRemovedAsync (
        const ServiceDescription& serviceDescription,
        const std::string& address,
        uint32_t interfaceIndex)
    {
        ignore (serviceDescription, address, interfaceIndex);
    }

    /**
     * Called when there was an error during browsing for a service.
     * Note: this call will be made from a background thread and wil not be synchronised.
     * @param error An Result indicating what problem occurred.
     */
    virtual void onBrowserErrorAsync (const Result& error) { ignore (error); }
};

} // namespace dnssd
