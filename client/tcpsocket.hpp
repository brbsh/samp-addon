#pragma once



#include "client.hpp"





class addonSocket
{

public:
	
	addonSocket();
	virtual ~addonSocket();

	bool Connect(std::string ip, unsigned short port);
	void Send(std::string data);

	boost::asio::ip::tcp::socket& getSocket()
	{
		boost::shared_lock<boost::shared_mutex> lockit(sockMutex);
		return socket;
	}

	boost::thread *getThreadInstance(bool recv)
	{
		return (recv) ? recvThreadInstance.get() : sendThreadInstance.get();
	}

	static void sendThread(boost::shared_ptr<boost::asio::ip::tcp::socket> socket);
	static void recvThread(boost::shared_ptr<boost::asio::ip::tcp::socket> socket);

private:

	boost::asio::io_service io_s;

	boost::shared_ptr<boost::thread> sendThreadInstance;
	boost::shared_ptr<boost::thread> recvThreadInstance;

	boost::shared_mutex sockMutex;
	boost::asio::ip::tcp::socket socket;
};