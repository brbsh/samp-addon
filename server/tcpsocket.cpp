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
//	boost::thread receive(boost::bind(&amxSocket::ReceiveThread));
}



amxSocket::~amxSocket()
{
	addonDebug("Socket deconstructor called");

	boost::mutex::scoped_lock lock(this->Mutex);
	this->Active = false;
	this->io.stop();
	lock.unlock();
}



bool amxSocket::IsClientConnected(int clientid)
{
	return (gPool->clientPool.count(clientid)) ? true : false;
}



void amxSocket::KickClient(int clientid)
{
	if(!this->IsClientConnected(clientid))
		return;

	boost::mutex::scoped_lock lock(this->Mutex);
	gPool->clientPool.find(clientid)->second.socketid->shutdown(boost::asio::socket_base::shutdown_both);
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
	int clientid;

	amxConnect connect;

	boost::mutex cMutex;
	boost::shared_ptr<boost::asio::ip::tcp::socket> socketid;
	boost::system::error_code error;

	addonDebug("Thread amxSocket::Thread() successfuly started");

	gSocket->io.run(error);

	if(error)
	{
		addonDebug("Cannot run I/O service");

		return;
	}

	boost::asio::ip::tcp::acceptor acceptor(gSocket->io, boost::asio::ip::tcp::endpoint(boost::asio::ip::address::from_string(gSocket->IP), gSocket->Port));

	addonDebug("TCP server started on %s:%i with max connections: %i", gSocket->IP.c_str(), gSocket->Port, gSocket->MaxClients);

	do
	{
		socketid = boost::shared_ptr<boost::asio::ip::tcp::socket>(new boost::asio::ip::tcp::socket(gSocket->io));
		acceptor.accept((*socketid));

		clientid = gSocket->MaxClients;

		for(unsigned int i = 0; i != gSocket->MaxClients; i++)
		{
			if(!gSocket->IsClientConnected(i))
			{
				clientid = i;

				break;
			}
		}

		if(clientid == gSocket->MaxClients)
		{
			addonDebug("Server is full");

			amxConnectError connect_error;

			connect_error.ip = socketid->remote_endpoint().address().to_string();
			connect_error.error.assign("Server is full");
			connect_error.errorCode = 1;

			boost::mutex::scoped_lock lock(cMutex);
			amxConnectErrorQueue.push(connect_error);
			lock.unlock();

			boost::this_thread::sleep(boost::posix_time::seconds(1));

			continue;
		}

		boost::mutex::scoped_lock lock(cMutex);
		gPool->clientPool[clientid].socketid = socketid;
		gPool->clientPool[clientid].ip = socketid->remote_endpoint().address().to_string();
		gPool->clientPool[clientid].Client.Auth = false;
		gPool->clientPool[clientid].Client.Serial = NULL;
		gPool->clientPool[clientid].Transfer.Active = false;
		lock.unlock();

		addonDebug("Incoming connection from %s (binded clientid: %i)", gPool->clientPool.find(clientid)->second.ip.c_str(), clientid);

		connect.clientID = clientid;
		connect.ip = gPool->clientPool.find(clientid)->second.ip;

		lock.lock();
		amxConnectQueue.push(connect);
		lock.unlock();

		boost::thread receive(boost::bind(&amxSocket::ReceiveThread, clientid));

		boost::this_thread::sleep(boost::posix_time::milliseconds(1));
	}
	while(gSocket->Active);

	acceptor.close();
}



void amxSocket::SendThread()
{
	processStruct data;

	boost::mutex sMutex;
	boost::system::error_code error;
	std::size_t length;

	addonDebug("Thread amxSocket::SendThread() successfuly started");

	do
	{
		if(!sendQueue.empty())
		{
			for(unsigned int i = 0; i < sendQueue.size(); i++)
			{
				boost::mutex::scoped_lock lock(sMutex);
				data = sendQueue.front();
				lock.unlock();

				if(!gSocket->IsClientConnected(data.clientID))
					continue;

				if(gPool->clientPool.find(data.clientID)->second.Transfer.Active)
				{
					boost::this_thread::sleep(boost::posix_time::seconds(1));
					
					continue;
				}

				lock.lock();
				sendQueue.pop();
				lock.unlock();

				addonDebug("Sending to %i: '%s'", data.clientID, data.data.c_str());

				length = gPool->clientPool.find(data.clientID)->second.socketid->write_some(boost::asio::buffer(data.data), error);

				addonDebug("Sent %i bytes to %i", length, data.clientID);

				boost::this_thread::sleep(boost::posix_time::milliseconds(1));
			}
		}

		boost::this_thread::sleep(boost::posix_time::milliseconds(1));
	}
	while(gSocket->Active);
}



void amxSocket::ReceiveThread(int clientid)
{
	char buffer[8192];

	processStruct data;
	
	boost::mutex rMutex;
	boost::system::error_code error;
	std::size_t length;

	addonDebug("Thread amxSocket::ReceiveThread(%i) successfuly started", clientid);

	while(gSocket->IsClientConnected(clientid))
	{
		if(gPool->clientPool.find(clientid)->second.Transfer.Active)
		{
			boost::this_thread::sleep(boost::posix_time::seconds(1));

			continue;
		}

		memset(buffer, NULL, sizeof buffer);
		length = gPool->clientPool.find(clientid)->second.socketid->read_some(boost::asio::buffer(buffer, sizeof buffer), error);

		if(error)
		{
			addonDebug("Client %i disconnected from server", clientid);

			amxDisconnect disconnect;

			disconnect.clientID = clientid;

			boost::mutex::scoped_lock lock(rMutex);
			amxDisconnectQueue.push(disconnect);

			gPool->clientPool.find(clientid)->second.socketid->close();
			gPool->clientPool.erase(clientid);
			lock.unlock();

			continue;
		}

		if(length > 0)
		{
			addonDebug("Received %i bytes from %i", length, clientid);

			data.clientID = clientid;
			data.data.assign(buffer, length);

			addonDebug("Received from %i: '%s'", clientid, data.data.c_str());

			boost::mutex::scoped_lock lock(rMutex);
			recvQueue.push(data);
			lock.unlock();
		}

		boost::this_thread::sleep(boost::posix_time::milliseconds(50));
	}
}