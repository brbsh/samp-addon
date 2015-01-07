#pragma once



#include "tcpsocket.h"





boost::asio::io_service gIOService;
boost::shared_ptr<amxSocket> gSocket;


extern boost::shared_ptr<amxDebug> gDebug;
extern boost::shared_ptr<amxPool> gPool;





amxSocket::amxSocket(std::string ip, int port, int maxclients)
{
	gDebug->Log("Socket constructor called");

	this->acceptThread = boost::shared_ptr<boost::thread>(new boost::thread(boost::bind(&amxSocket::AcceptThread, ip, port, maxclients)));
	this->sendThread = boost::shared_ptr<boost::thread>(new boost::thread(boost::bind(&amxSocket::SendThread)));
//	boost::thread receive(boost::bind(&amxSocket::ReceiveThread));
}



amxSocket::~amxSocket()
{
	gDebug->Log("Socket destructor called");

	this->acceptThread->interruption_requested();
	this->sendThread->interruption_requested();
	gIOService.stop();
}



/*bool amxSocket::IsClientConnected(int clientid)
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
}*/



void amxSocket::AcceptThread(std::string ip, int port, int maxClients)
{
	int clientid;

	boost::mutex aMutex;
	boost::shared_ptr<boost::asio::ip::tcp::socket> socketid;
	boost::system::error_code error;

	gDebug->Log("Thread amxSocket::Thread() successfuly started");

	gIOService.run(error);

	if(error)
	{
		gDebug->Log("Cannot run I/O service: %s (Error code: %i)", error.message().c_str(), error.value());

		return;
	}

	boost::asio::ip::tcp::acceptor acceptor(gIOService, boost::asio::ip::tcp::endpoint(boost::asio::ip::address::from_string(ip), port));

	gDebug->Log("TCP server started on %s:%i with max connections: %i", ip.c_str(), port, maxClients);

	do
	{
		socketid = boost::shared_ptr<boost::asio::ip::tcp::socket>(new boost::asio::ip::tcp::socket(gIOService));
		acceptor.accept((*socketid));

		clientid = maxClients;

		for(unsigned int i = 0; i != maxClients; i++)
		{
			/*if(!gSocket->IsClientConnected(i))
			{
				clientid = i;

				break;
			}*/
		}

		if(clientid == maxClients)
		{
			gDebug->Log("Server is full");

			/*amxConnectError connect_error;

			connect_error.ip = socketid->remote_endpoint().address().to_string();
			connect_error.error.assign("Server is full");
			connect_error.errorCode = 1;

			aMutex.lock();
			amxConnectErrorQueue.push(connect_error);
			aMutex.unlock();*/

			socketid->shutdown(boost::asio::socket_base::shutdown_both);
			socketid->close();
			socketid.reset();

			boost::this_thread::sleep_for(boost::chrono::milliseconds(10));

			continue;
		}

		/*boost::mutex::scoped_lock lock(cMutex);
		gPool->clientPool[clientid].socketid = socketid;
		gPool->clientPool[clientid].ip = socketid->remote_endpoint().address().to_string();
		gPool->clientPool[clientid].Client.Auth = false;
		gPool->clientPool[clientid].Client.Serial = NULL;
		gPool->clientPool[clientid].Transfer.Active = false;
		lock.unlock();

		gDebug->Log("Incoming connection from %s (binded clientid: %i)", gPool->clientPool.find(clientid)->second.ip.c_str(), clientid);

		connect.clientID = clientid;
		connect.ip = gPool->clientPool.find(clientid)->second.ip;

		lock.lock();
		amxConnectQueue.push(connect);
		lock.unlock();

		boost::thread receive(boost::bind(&amxSocket::ReceiveThread, clientid));*/

		boost::this_thread::sleep_for(boost::chrono::milliseconds(10));
	}
	while(gPool->pluginInit.load());

	acceptor.close();
}



void amxSocket::SendThread()
{
	//processStruct data;

	boost::mutex sMutex;
	boost::system::error_code error;
//	std::size_t length;

	gDebug->Log("Thread amxSocket::SendThread() successfuly started");

	do
	{
		boost::this_thread::disable_interruption di;

		/*if(!sendQueue.empty())
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
		}*/

		boost::this_thread::restore_interruption re(di);
		boost::this_thread::sleep_for(boost::chrono::milliseconds(10));
	}
	while(gPool->pluginInit.load());
}



/*void amxSocket::ReceiveThread(int clientid)
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
}*/