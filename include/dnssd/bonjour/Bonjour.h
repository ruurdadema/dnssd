#pragma once

#ifdef _WIN32
#define _WINSOCKAPI_ // Prevents inclusion of winsock.h in windows.h
#include <winsock2.h>
#include <Ws2tcpip.h>
#else
#include <arpa/inet.h>
#endif

#include <dns_sd.h>
