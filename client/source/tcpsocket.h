#pragma once



#include "addon.h"





struct fileTransfer
{
	bool is;
	bool send;
	std::string file;
	long file_length;
	std::string remote_file;
};



class addonSocket
{

public:

	static DWORD SendThread(void *lpParam);
	static DWORD ReceiveThread(void *lpParam);

	struct fileTransfer Transfer;
	
	bool threadActive;
	bool transfer;

	addonSocket();
	~addonSocket();

	void Connect(std::string address, int port);
	void Send(std::string data);
	void Close();

	int GetInstance();

private:

	HANDLE sendHandle;
	HANDLE receiveHandle;

	int socketHandle;

	int Create();
	int Nonblock();
};