#pragma once



#include "tcpsocket.h"





amxSocket *gSocket;

std::queue<processStruct> sendQueue;
std::queue<processStruct> recvQueue;


extern logprintf_t logprintf;

extern amxPool *gPool;

extern std::queue<amxConnect> amxConnectQueue;
extern std::queue<amxConnectError> amxConnectErrorQueue;
extern std::queue<amxDisconnect> amxDisconnectQueue;





amxSocket::amxSocket(std::string ip, int port, int maxclients)
{
	addonDebug("Socket constructor called");

	boost::mutex::scoped_lock lock(this->Mutex);
	this->Active = true;
	lock.unlock();

	this->IP = ip;
	this->Port = port;
	this->MaxClients = maxclients;

	boost::thread accept(boost::bind(&amxSocket::Thread));
	boost::thread send(boost::bind(&amxSocket::SendThread));
	boost::thread receive(boost::bind(&amxSocket::ReceiveThread));
}



amxSocket::~amxSocket()
{
	addonDebug("Socket deconstructor called");

	boost::mutex::scoped_lock lock(this->Mutex);
	this->Active = false;
	lock.unlock();
}



bool amxSocket::IsClientConnected(int clientid)
{
	return (gSocket->Socket.count(clientid)) ? true : false;
}



void amxSocket::KickClient(int clientid)
{
	if(!this->IsClientConnected(clientid))
		return;

	cliPool client = gPool->clientPool[clientid];
	client.Active = false;
	
	boost::mutex::scoped_lock lock(this->Mutex);
	gPool->clientPool[clientid] = client;
	lock.unlock();
}



void amxSocket::Send(int clientid, std::string data)
{
	if(!this->IsClientConnected(clientid))
		return;

	processStruct pushme;

	pushme.clientID = clientid;
	pushme.data = data;

	boost::mutex::scoped_lock lock(this->Mutex);
	sendQueue.push(pushme);
	lock.unlock();
}



void amxSocket::Thread()
{
	addonDebug("Thread amxSocket::Thread() successfuly started");

	boost::asio::io_service io;
	boost::system::error_code error;

	boost::asio::ip::tcp::acceptor acceptor(io, boost::asio::ip::tcp::endpoint(boost::asio::ip::address::from_string(gSocket->IP), gSocket->Port));

	int sockid = NULL;

	do
	{
		if(sockid >= gSocket->MaxClients)
		{
			boost::this_thread::sleep(boost::posix_time::milliseconds(1));

			continue;
		}

		// sockid = find_free_slot

		gSocket->Socket[sockid] = boost::shared_ptr<boost::asio::ip::tcp::socket>(new boost::asio::ip::tcp::socket(io));
		acceptor.accept((*gSocket->Socket[sockid]));

		addonDebug("Incoming connection from %s", gSocket->Socket[sockid]->remote_endpoint().address().to_string().c_str());
		addonDebug("IP %s got clientid %i", gSocket->Socket[sockid]->remote_endpoint().address().to_string().c_str(), sockid);

		sockid++;

		boost::this_thread::sleep(boost::posix_time::milliseconds(1));
	}
	while(gSocket->Active);

	acceptor.close();
}



void amxSocket::SendThread()
{
	addonDebug("Thread amxSocket::SendThread() successfuly started");

	processStruct data;

	boost::mutex sMutex;
	std::size_t length;

	do
	{
		if(!sendQueue.empty())
		{
			for(unsigned int i = 0; i < sendQueue.size(); i++)
			{
				boost::mutex::scoped_lock lock(sMutex);
				data = sendQueue.front();
				sendQueue.pop();
				lock.unlock();

				if(!gSocket->IsClientConnected(data.clientID))
					continue;

				addonDebug("Sending to %i: '%s'", data.clientID, data.data.c_str());

				length = gSocket->Socket[data.clientID]->write_some(boost::asio::buffer(data.data));

				addonDebug("Sent %i bytes to %i", length, data.clientID);

				boost::this_thread::sleep(boost::posix_time::milliseconds(1));
			}
		}

		boost::this_thread::sleep(boost::posix_time::milliseconds(1));
	}
	while(gSocket->Active);
}



void amxSocket::ReceiveThread()
{
	addonDebug("Thread amxSocket::ReceiveThread() successfuly started");

	char buffer[8192];

	processStruct data;

	boost::mutex rMutex;
	boost::system::error_code error;
	std::size_t length;

	do
	{
		for(boost::unordered_map<int, boost::shared_ptr<boost::asio::ip::tcp::socket> >::iterator i = gSocket->Socket.begin(); i != gSocket->Socket.end(); i++)
		{
			memset(buffer, NULL, sizeof buffer);
			length = i->second->read_some(boost::asio::buffer(buffer, sizeof buffer), error);

			if(error == boost::asio::error::eof)
			{
				//disconnected
				addonDebug("Closed connection");

				continue;
			}

			if(length > 0)
			{
				addonDebug("Received %i bytes from %i", length, i->first);

				data.clientID = i->first;
				data.data.assign(buffer, length);

				addonDebug("Received from %i: '%s'", i->first, data.data.c_str());

				boost::mutex::scoped_lock lock(rMutex);
				recvQueue.push(data);
				lock.unlock();
			}
			boost::this_thread::sleep(boost::posix_time::milliseconds(1));
		}

		boost::this_thread::sleep(boost::posix_time::milliseconds(1));
	}
	while(gSocket->Active);
}