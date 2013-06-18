#pragma once



#ifdef WIN32
	#define WIN32_LEAN_AND_MEAN

	#include <Windows.h>
	#include <winsock2.h>
	#include <ws2tcpip.h>

	#pragma comment(lib, "ws2_32.lib")
#else
	#include <sys/types.h>
	#include <sys/socket.h>
	#include <arpa/inet.h>
	#include <netdb.h>
	#include <fcntl.h>
	#include <unistd.h>
	#include <pthread.h>
#endif


#include <algorithm>
#include <fstream>
#include <iostream>
#include <list>
#include <map>
#include <queue>
#include <sstream>
#include <string>


#include "..\includes\sdk\amx\amx.h"
#include "..\includes\sdk\plugincommon.h"

#include "mutex.h"
#include "natives.h"
#include "pool.h"
#include "process.h"
#include "string.h"
#include "tcpsocket.h"
#include "thread.h"
#include "transfer.h"



#ifdef WIN32
	#define SLEEP(x) { Sleep(x); }
#else
	#define SLEEP(x) { usleep(x * 1000); }

	typedef unsigned long DWORD;
	typedef unsigned int UINT;
#endif





typedef void (*logprintf_t)(char* format, ...);



struct amxConnect
{
	int clientID;
	std::string ip;
};



struct amxConnectError
{
	std::string ip;
	int errorCode;
	std::string error;
};



struct amxDisconnect
{
	int clientID;
};



struct amxKey
{
	int clientID;
	char keys[256];
};



struct amxScreenshot
{
	int clientID;
	char name[256];
};



struct amxFileReceive
{
	int clientid;
	std::string file;
};



struct amxFileSend
{
	int clientid;
	std::string file;
};



struct amxFileError
{
	int io;
	int clientid;
	std::string file;
	std::string error;
	int errorCode;
};