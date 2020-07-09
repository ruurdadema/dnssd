//
// Created by Ruurd Adema on 28/06/2020.
//

#pragma once

#include "ScopedDNSServiceRef.h"

namespace dnssd {

    class SharedConnection
    {
    public:
        SharedConnection();
        DNSServiceRef serviceRef() const noexcept { return mServiceRef; }

    private:
        ScopedDNSServiceRef mServiceRef;
    };

} // namespace dnssd
