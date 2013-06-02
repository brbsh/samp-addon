#pragma once

#define WIN32_LEAN_AND_MEAN 
#define _CRT_SECURE_NO_WARNINGS

#include <Windows.h>

#include <string>
#include <fstream>
#include <time.h>





typedef void (WINAPI *addonLoader)();


void addonDebug(char *text, ...);


DWORD __stdcall addon_load_thread(LPVOID lpParam);