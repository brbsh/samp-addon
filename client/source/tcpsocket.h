#pragma once



#include "addon.h"





class addonSocket
{

public:

	static DWORD SendThread(void *lpParam);
	static DWORD ReceiveThread(void *lpParam);
	
	bool threadActive;

	addonSocket();
	~addonSocket();

	void Connect(std::string address, int port);
	void Send(std::string data);
	void Close();

private:

	HANDLE sendHandle;
	HANDLE receiveHandle;

	int socketHandle;

	int Create();
	int Nonblock();
};