#pragma once



#include "addon.h"





class amxTransfer
{

public:

	static void SendThread(int clientid, std::string file_name, std::string remote_file_name);
	static void ReceiveThread(int clientid, std::string file_name, std::size_t length);
};