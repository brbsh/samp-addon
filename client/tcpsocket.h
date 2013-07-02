#pragma once



#include "addon.h"





class addonSocket
{

public:

	boost::mutex Mutex;
	boost::shared_ptr<boost::asio::ip::tcp::socket> Socket;

	static void Thread();
	
	bool threadActive;

	addonSocket();
	~addonSocket();

	void Send(std::string data);
};