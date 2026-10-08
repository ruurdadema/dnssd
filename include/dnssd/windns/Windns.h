#pragma once

#if _WIN32 && USE_WINDNS
#ifndef _WINSOCKAPI_
#define _WINSOCKAPI_ // Prevents inclusion of winsock.h in windows.h
#endif
#include <Ws2tcpip.h>
#include <winsock2.h>
#include <windows.h>
#include <windns.h>
#include <winerror.h>
#else
#error Set '-DUSE_WINDNS=ON' as a cmake option during generation to use Windns.
#endif

#include <chrono>

namespace dnssd
{

/// Maximum time to wait for a WinDNS async callback to complete during shutdown.
constexpr std::chrono::seconds kWindnsCallbackTimeout { 5 };

} // namespace dnssd
