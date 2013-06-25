#pragma once

#define WIN32_LEAN_AND_MEAN 
#define _CRT_SECURE_NO_WARNINGS



#include <fstream>
#include <string>
#include <time.h>
#include <windows.h>

#include <boost/thread.hpp>





typedef void (WINAPI *addonLoader)();





void addon_load_thread();
void addonDebug(char *text, ...);