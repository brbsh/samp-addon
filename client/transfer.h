#pragma once



#include "addon.h"





class addonTransfer
{

public:

	static void SendThread(int socketid);
	static void ReceiveThread(int socketid);
};