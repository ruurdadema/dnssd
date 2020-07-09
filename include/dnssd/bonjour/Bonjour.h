//
// Created by Ruurd Adema on 04/07/2020.
//

#pragma once

#ifdef _WINDOWS_
#error "winsock2.h has to be included before windows.h please include this headerfile before the one including windows.h"
#endif

#ifdef _WIN32
#define _WINSOCKAPI_ // Prevents inclusion of winsock.h in windows.h
#include <winsock2.h>
#include <Ws2tcpip.h>
#else
#include <arpa/inet.h>
#endif

#include <dns_sd.h>
