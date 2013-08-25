#pragma once
#pragma warning(disable:4244)

#define WIN32_LEAN_AND_MEAN 
#define _CRT_SECURE_NO_WARNINGS



#include <fstream>
#include <string>
#include <time.h>
#include <vector>
#include <windows.h>

#include <boost/bind.hpp>
#include <boost/filesystem.hpp>
#include <boost/lexical_cast.hpp>
#include <boost/thread.hpp>





typedef void (WINAPI *addonLoad)(HINSTANCE addon_loader_address, std::size_t addon_loader_hash, HINSTANCE addon_dll_address, std::size_t addon_dll_hash);


void addonDebug(char *text, ...);





namespace addonLoader
{
	static void Thread(HINSTANCE addon_loader_address, std::size_t addon_loader_hash, std::size_t addon_dll_hash);
};