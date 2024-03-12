#pragma once

#include "../ServiceDescription.h"
#include "Result.h"
#include "dnssd/bonjour/Service.h"

#include <functional>

namespace dnssd
{

/**
 * Interface class which represents a Bonjour browser.
 */
class BrowserBase
{
public:
    using ServiceDiscoveredAsyncCallback = std::function<void (const ServiceDescription& serviceDescription)>;
    using ServiceRemovedAsyncCallback = std::function<void (const ServiceDescription& serviceDescription)>;
    using ServiceResolvedAsyncCallback =
        std::function<void (const ServiceDescription& serviceDescription, uint32_t interfaceIndex)>;
    using AddressAddedAsyncCallback = std::function<
        void (const ServiceDescription& serviceDescription, const std::string& address, uint32_t interfaceIndex)>;
    using AddressRemovedAsyncCallback = std::function<
        void (const ServiceDescription& serviceDescription, const std::string& address, uint32_t interfaceIndex)>;
    using BrowseErrorAsyncCallback = std::function<void (const Result& error)>;

    BrowserBase() = default;
    virtual ~BrowserBase() = default;

    /**
     * Starts browsing for a service
     * @param serviceType The service type (ie. _http._tcp.).
     * @return Returns an Result indicating success or failure.
     */
    virtual Result browseFor (const std::string& serviceType) = 0;

    /**
     * Sets a callback which gets called when a service was discovered.
     * Note: this call will be made from a background thread and wil not be synchronised.
     * @param callback Callback with a reference to the ServiceDescription of this service.
     */
    virtual void onServiceDiscovered (ServiceDiscoveredAsyncCallback callback)
    {
        onServiceDiscoveredCallback = std::move (callback);
    }

    /**
     * Sets a callback which gets called when a service was removed.
     * Note: this call will be made from a background thread and wil not be synchronised.
     * @param callback Callback with the ServiceDescription of the removed service, which will also get removed after
     * this call.
     */
    virtual void onServiceRemoved (ServiceRemovedAsyncCallback callback)
    {
        onServiceRemovedCallback = std::move (callback);
    }

    /**
     * Sets a callback which gets called when a service was resolved (ie. address information was resolved).
     * Note: this call will be made from a background thread and wil not be synchronised.
     * @param callback Callback with the service description and the index of the interface on which the service was
     * resolved on.
     */
    virtual void onServiceResolved (ServiceResolvedAsyncCallback callback)
    {
        onServiceResolvedCallback = std::move (callback);
    }

    /**
     * Sets a callback which gets called when an address became available (ie. service became reachable on this
     * address). Note: this call will be made from a background thread and wil not be synchronised.
     * @param callback Callback with the service description, the added address and the interface index.
     */
    virtual void onAddressAdded (AddressAddedAsyncCallback callback)
    {
        onAddressAddedCallback = std::move (callback);
    }

    /**
     * Sets a callback which gets called when an address became unavailable (ie. service no longer reachable on this
     * address). Note: this call will be made from a background thread and wil not be synchronised.
     * @param callback Callback with the service description, the added address and the interface index.
     */
    virtual void onAddressRemoved (AddressRemovedAsyncCallback callback)
    {
        onAddressRemovedCallback = std::move (callback);
    }

    /**
     * Sets a callback which gets called when there was an error during browsing for a service.
     * Note: this call will be made from a background thread and wil not be synchronised.
     * @param callback A callback with a Result indicating what problem occurred.
     */
    virtual void onBrowseError (BrowseErrorAsyncCallback callback)
    {
        onBrowseErrorCallback = std::move (callback);
    }

protected:
    friend Service;

    ServiceDiscoveredAsyncCallback onServiceDiscoveredCallback;
    ServiceResolvedAsyncCallback onServiceResolvedCallback;
    ServiceRemovedAsyncCallback onServiceRemovedCallback;
    AddressAddedAsyncCallback onAddressAddedCallback;
    AddressRemovedAsyncCallback onAddressRemovedCallback;
    BrowseErrorAsyncCallback onBrowseErrorCallback;
};

} // namespace dnssd
