#pragma once



#include "addon.h"





class addonTransfer
{

public:

	static void SendThread(std::string file_name);
	static void ReceiveThread(std::string file_name, std::size_t file_length);
};