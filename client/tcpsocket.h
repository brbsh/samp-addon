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

	boost::mutex Mutex;

	static void SendThread(int socketid);
	static void ReceiveThread(int socketid);

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

	int socketHandle;

	int Create();
	int Nonblock();
};