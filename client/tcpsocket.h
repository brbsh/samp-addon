#pragma once



#include "client.h"





class addonSocket
{

public:
	
	boost::asio::io_service IOService;

	addonSocket();
	virtual ~addonSocket();

	void Connect(std::string ip, unsigned short port);
	void Send(std::string data);

	boost::asio::ip::tcp::socket *getSocket() const
	{
		return socket.get();
	}

	boost::mutex *getMutexInstance() const
	{
		return mutexInstance.get();
	}

	boost::thread *getThreadInstance(bool recv) const
	{
		return (recv) ? recvThreadHandle.get() : sendThreadHandle.get();
	}

	static void sendThread(addonSocket *sock);
	static void recvThread(addonSocket *sock);

private:

	boost::shared_ptr<boost::mutex> mutexInstance;
	boost::shared_ptr<boost::thread> sendThreadHandle;
	boost::shared_ptr<boost::thread> recvThreadHandle;

	boost::shared_ptr<boost::asio::ip::tcp::socket> socket;
};