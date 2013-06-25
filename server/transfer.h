#pragma once



#include "addon.h"





class amxTransfer
{

public:

	boost::mutex Mutex;

	static void SendThread(int clientid);
	static void ReceiveThread(int clientid);
};