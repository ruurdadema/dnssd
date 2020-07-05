/*
  ==============================================================================

    Debug.cpp
    Created: 24 Jun 2020 2:17:16pm
    Author:  ruurd

  ==============================================================================
*/

#include <dnssd/common/Debug.h>
#include <string>
#include <iostream>
#ifdef _WIN32
#include <Windows.h>
#endif

void dnssd_debug_log(const std::string& str)
{
#ifdef _WIN32
	OutputDebugStringA(str.c_str());
#else
	std::cout << str;
#endif
}
