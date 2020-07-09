//
// Created by Ruurd Adema on 28/06/2020.
//

#pragma once

#include "ScopedDnsServiceRef.h"

namespace dnssd {

    class SharedConnection
    {
    public:
        SharedConnection();
        DNSServiceRef serviceRef() const noexcept { return mServiceRef.serviceRef(); }

    private:
        ScopedDnsServiceRef mServiceRef;
    };

} // namespace dnssd
