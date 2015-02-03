#pragma once
#pragma comment(lib, "urlmon.lib")
#pragma warning(disable:4996 4244 4700)



#define ADDON_UPDATER_VERSION	(20150203)



#define WIN32_LEAN_AND_MEAN
#include <Windows.h>


#include <boost/crc.hpp>
#include <boost/filesystem.hpp>


#include <iostream>
#include <string>
#include <TlHelp32.h>
#include <UrlMon.h>

