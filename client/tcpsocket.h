#pragma once



#include "addon.h"





class addonSocket
{

public:

	static void Thread();
	static void SendThread();
	static void ReceiveThread();

	addonSocket();
	~addonSocket();

	void Send(std::string data);

	bool threadActive;

	boost::mutex Mutex;
	boost::shared_ptr<boost::asio::ip::tcp::socket> Socket;
};