#pragma once

#include "addon.h"




class addonSocket
{
public:
	int socket_handle;
	bool socket_active;
	HANDLE send_thread;
	HANDLE receive_thread;

	addonSocket();
	~addonSocket();

	int Start();
	void Connect(std::string address, int port);
	int set_nonblocking_socket(int fd);
	void Send(std::string data);
	void Close();
};