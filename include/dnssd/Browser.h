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
