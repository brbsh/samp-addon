#pragma once

#define WIN32_LEAN_AND_MEAN 
#define _CRT_SECURE_NO_WARNINGS



#include <fstream>
#include <string>
#include <time.h>
#include <Windows.h>





typedef void (WINAPI *addonLoader)();



void addonDebug(char *text, ...);
DWORD addon_load_thread(void *lpParam);