//
// Created by ruurd on 11/05/2022.
//

#include "dnssd/bonjour/Bonjour.h"

bool dnssd::isBonjourServiceRunning()
{
    uint32_t version;
    uint32_t size = sizeof (version);
    DNSServiceErrorType error = DNSServiceGetProperty (kDNSServiceProperty_DaemonVersion, &version, &size);

    return error == kDNSServiceErr_NoError;
}
