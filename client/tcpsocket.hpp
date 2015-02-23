#pragma once



#include "client.hpp"





class addonSocket
{

public:
	
	addonSocket();
	virtual ~addonSocket();

	bool connectTo(std::string ip, unsigned short port);
	void disconnect();
	void writeTo(std::string data);

	boost::asio::ip::tcp::socket& getSocket()
	{
		boost::shared_lock<boost::shared_mutex> lockit(sockMutex);
		return socket;
	}

	boost::thread *getThreadInstance()
	{
		return recvThreadInstance.get();
	}

	bool isConnected() const
	{
		return connected;
	}

	static void connectionThread(addonSocket *instance);
	static void recvThread(addonSocket *instance);

private:

	bool connected;

	boost::asio::io_service io_s;
	boost::shared_ptr<boost::thread> connectionThreadInstance;
	boost::shared_ptr<boost::thread> recvThreadInstance;

	boost::shared_mutex sockMutex;
	boost::asio::ip::tcp::socket socket;
};