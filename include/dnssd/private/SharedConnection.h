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
        operator DNSServiceRef() const noexcept { return mServiceRef; }

    private:
        ScopedDNSServiceRef mServiceRef;
    };
}
