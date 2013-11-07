#pragma once



#include "client.h"





class addonSocket
{

public:
	
	addonSocket();
	virtual ~addonSocket();

	bool Connect(std::string ip, UINT port);
	void Send(std::string data);

	boost::asio::ip::tcp::socket *getSocket() const
	{
		return socket.get();
	}

	boost::mutex *getMutexInstance(bool recv) const
	{
		return (recv) ? mutexRecvInstance.get() : mutexSendInstance.get();
	}

	boost::thread *getThreadInstance(bool recv) const
	{
		return (recv) ? recvThreadInstance.get() : sendThreadInstance.get();
	}

	static void sendThread();
	static void recvThread();

private:

	boost::shared_ptr<boost::mutex> mutexSendInstance;
	boost::shared_ptr<boost::mutex> mutexRecvInstance;

	boost::shared_ptr<boost::thread> sendThreadInstance;
	boost::shared_ptr<boost::thread> recvThreadInstance;

	boost::shared_ptr<boost::asio::ip::tcp::socket> socket;
};