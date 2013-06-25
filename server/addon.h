#pragma once



#ifdef WIN32
	#define WIN32_LEAN_AND_MEAN

	#include <windows.h>
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

//#include <boost/asio.hpp>
#include <boost/thread.hpp>
#include <boost/thread/mutex.hpp>
#include <boost/unordered_map.hpp>

#define HAVE_STDINT_H

#include "SDK/plugin.h"

//#include "mutex.h"
#include "natives.h"
#include "pool.h"
#include "process.h"
#include "string.h"
#include "tcpsocket.h"
//#include "thread.h"
#include "transfer.h"



#define arguments(a) \
	!(params[0] != (a << 2))





typedef void (*logprintf_t)(char *format, ...);



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