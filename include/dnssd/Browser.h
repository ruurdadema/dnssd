//
// Created by Ruurd Adema on 28/06/2020.
//

#pragma once

#include "ServiceDescription.h"
#include "dnssd/common/Error.h"
#include "dnssd/common/CommonBrowserInterface.h"

#include <string>
#include <thread>
#include <atomic>

namespace dnssd {

    class Browser
    {
    public:
        using Listener = CommonBrowserInterface::Listener;

        explicit Browser(const Listener& listener);
        Error browseFor(const std::string& service);

    private:
        std::unique_ptr<CommonBrowserInterface> impl;
    };

} // namespace dnssd
