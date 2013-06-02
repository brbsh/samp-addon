#pragma once

#include "addon.h"




class addonSocket
{

public:

	bool active;
	int socketHandle;
	HANDLE sendHandle;
	HANDLE receiveHandle;

	addonSocket();
	~addonSocket();

	int Create();
	void Connect(std::string address, int port);
	int set_nonblocking_socket();
	void Send(std::string data);
	void Close();
};