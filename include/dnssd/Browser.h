//
// Created by Ruurd Adema on 28/06/2020.
//

#pragma once

#include "ServiceDescription.h"
#include "dnssd/common/Error.h"
#include "dnssd/bonjour/BonjourBrowser.h"

#include <string>
#include <thread>
#include <atomic>

namespace dnssd {

    using Browser = BonjourBrowser;

} // namespace dnssd
