#pragma once



#include "addon.h"





class amxTransfer
{

public:

	static void SendThread(int clientid);
	static void ReceiveThread(int clientid);
};