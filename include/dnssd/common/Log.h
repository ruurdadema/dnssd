/*
  ==============================================================================

    Debug.h
    Created: 24 Jun 2020 2:07:02pm
    Author:  ruurd

  ==============================================================================
*/

#pragma once

#ifdef DNSSD_DEBUG
#include <sstream>

#ifdef _WIN32
#include <Windows.h>
#endif

#ifndef DNSSD_LOG_DEBUG

void dnssd_debug_log(const std::string& str);
#define DNSSD_LOG_DEBUG(msg) { std::ostringstream tmp; tmp << msg; dnssd_debug_log(tmp.str()); }

#endif
#else

#define DNSSD_LOG_DEBUG(msg) {}

#endif
