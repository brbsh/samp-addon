#pragma once



#include "tcpsocket.h"





extern amxPool *gPool;

extern logprintf_t logprintf;


extern std::queue<amxConnect> amxConnectQueue;
extern std::queue<amxConnectError> amxConnectErrorQueue;
extern std::queue<amxDisconnect> amxDisconnectQueue;


amxSocket *gSocket;

std::queue<processStruct> sendQueue;
std::queue<processStruct> recvQueue;





amxSocket::amxSocket(int port, int maxclients)
{
	logprintf("Addon: Socket constructor called");

	boost::mutex::scoped_lock lock(this->Mutex);
	this->Active = true;
	lock.unlock();

	this->Port = port;
	this->MaxClients = maxclients;

	boost::thread accept(&amxSocket::Thread);
}



amxSocket::~amxSocket()
{
	logprintf("Addon: Socket deconstructor called");

	boost::mutex::scoped_lock lock(this->Mutex);
	this->Active = false;
	lock.unlock();
}



void amxSocket::Thread()
{
	boost::asio::io_service io;
	boost::system::error_code error;

	boost::asio::ip::tcp::acceptor acceptor(io, boost::asio::ip::tcp::endpoint(boost::asio::ip::tcp::v4(), gSocket->Port));

	int sockid = NULL;

	do
	{
		if(sockid >= gSocket->MaxConnections)
		{
			boost::this_thread::sleep(boost::posix_time::milliseconds(1));

			continue;
		}

		// sockid = find_free_slot

		gSocket->Socket[sockid] = boost::shared_ptr<boost::asio::ip::tcp::socket>(new boost::asio::ip::tcp::socket(io));
		acceptor.accept(*gSocket->Socket[sockid]);

		boost::thread client(boost::bind(&amxSocket::ClientThread, sockid));

		boost::this_thread::sleep(boost::posix_time::milliseconds(1));
	}
	while(gSocket->Active);

	acceptor.close();
}



void amxSocket::ClientThread(int clientid)
{
	boost::system::error_code error;

	char buffer[65535];
	int length;

	do
	{
		length = gSocket->Socket[clientid]->read_some(boost::asio::buffer(buffer, 65535), error);

		if(length > 0)
		{
			//

			continue;
		}
	}
	while(!error || (error != boost::asio::error::eof));

	gSocket->Socket[clientid]->close();
	gSocket->Socket.quick_erase(gSocket->Socket.find(clientid));
}