#include <dnssd/bonjour/SharedConnection.h>
#include <dnssd/common/Error.h>
#include <dnssd/common/Log.h>

dnssd::SharedConnection::SharedConnection()
{
    DNSServiceRef ref = nullptr;
    if (auto error = dnssd::Error (DNSServiceCreateConnection (&ref)))
        DNSSD_LOG_DEBUG (error.description())
    else
        mServiceRef = ref; // From here on the ref is under RAII inside a ScopedDnsServiceRef class
}
