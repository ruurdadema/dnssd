//
// Created by Ruurd Adema on 28/06/2020.
//

#include <dnssd/internal/SharedConnection.h>
#include <dnssd/internal/Error.h>
#include <dnssd/internal/Debug.h>

dnssd::SharedConnection::SharedConnection()
{
    DNSServiceRef ref = nullptr;
    if (auto error = dnssd::Error(DNSServiceCreateConnection(&ref)))
    {
        DNSSD_LOG_DEBUG(error.description())
        throw Exception(error);
    }
    mServiceRef = ref; // From here on the ref is under RAII inside a ScopedDNSServiceRef class
}
