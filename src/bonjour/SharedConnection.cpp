//
// Created by Ruurd Adema on 28/06/2020.
//

#include <dnssd/bonjour/SharedConnection.h>
#include <dnssd/common/Error.h>
#include <dnssd/common/Debug.h>

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
