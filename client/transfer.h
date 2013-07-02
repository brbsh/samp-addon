#pragma once



#include "addon.h"





class addonTransfer
{

public:

	static void SendThread(std::string file_name);
	static void ReceiveThread(std::string file_name, long int file_length);
};