#pragma once



#include "tcpsocket.h"





boost::asio::io_service gIOService;
boost::shared_ptr<amxSocket> gSocket;


extern boost::shared_ptr<amxDebug> gDebug;
extern boost::shared_ptr<amxPool> gPool;





amxSocket::amxSocket(std::string ip, unsigned int port, unsigned int maxclients)
{
	gDebug->Log("Socket constructor called");

	this->acceptThread = boost::shared_ptr<boost::thread>(new boost::thread(boost::bind(&amxSocket::AcceptThread, ip, port, maxclients)));
	this->acceptMutex = boost::shared_ptr<boost::mutex>(new boost::mutex());
}



amxSocket::~amxSocket()
{
	gDebug->Log("Socket destructor called");

	this->acceptThread->interruption_requested();
	gIOService.stop();
}



bool amxSocket::IsClientConnected(unsigned int clientid)
{
	return (gPool->clientPool.count(clientid) > 0) ? true : false;
}



void amxSocket::KickClient(unsigned int clientid)
{
	if(!this->IsClientConnected(clientid))
		return;

	boost::unordered_map<unsigned int, amxPool::clientPoolS>::iterator f;

	f = gPool->clientPool.find(clientid);
	gPool->clientPool.find(clientid)->second.sockid->cancel();
	gPool->clientPool.find(clientid)->second.sockid->shutdown(boost::asio::socket_base::shutdown_both);
	gPool->clientPool.find(clientid)->second.sockid->close();
	gPool->clientPool.quick_erase(f);

	gDebug->Log("Client %i was kicked", clientid);
}



void amxSocket::AcceptThread(std::string ip, unsigned int port, unsigned int maxClients)
{
	int clientid;

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
		acceptor.accept(*socketid);

		boost::this_thread::disable_interruption di;

		clientid = maxClients;

		for(unsigned int i = 0; i != maxClients; i++)
		{
			if(!gSocket->IsClientConnected(i))
			{
				clientid = i;

				break;
			}
		}

		if(clientid == maxClients)
		{
			gDebug->Log("Cannot accept connection from %s : Server is full", socketid->remote_endpoint().address().to_string().c_str());

			/*amxConnectError connect_error;

			connect_error.ip = socketid->remote_endpoint().address().to_string();
			connect_error.error.assign("Server is full");
			connect_error.errorCode = 1;

			aMutex.lock();
			amxConnectErrorQueue.push(connect_error);
			aMutex.unlock();*/

			socketid->cancel();
			socketid->shutdown(boost::asio::socket_base::shutdown_both);
			socketid->close();
			socketid.reset();

			boost::this_thread::restore_interruption re(di);
			boost::this_thread::sleep_for(boost::chrono::milliseconds(10));

			continue;
		}

		gPool->clientPool[clientid].ip = socketid->remote_endpoint().address().to_string();
		gPool->clientPool[clientid].sockid = socketid;
		gPool->clientPool[clientid].sID = NULL;

		gDebug->Log("Incoming connection from %s (binded clientid: %i)", gPool->clientPool.find(clientid)->second.ip.c_str(), clientid);

		/*connect.clientID = clientid;
		connect.ip = gPool->clientPool.find(clientid)->second.ip;

		lock.lock();
		amxConnectQueue.push(connect);
		lock.unlock();*/

		boost::this_thread::restore_interruption re(di);
		boost::this_thread::sleep_for(boost::chrono::milliseconds(10));
	}
	while(gPool->pluginInit.load());

	acceptor.close();
}



void amxSocket::asyncWrite(unsigned int clientid, std::string data)
{
	// checks

	gPool->clientPool.find(clientid)->second.sockid->async_send(boost::asio::buffer(data, data.length()), boost::bind(&amxSocket::asyncWriteCallback, clientid, boost::asio::placeholders::error));
}



void amxSocket::asyncWriteCallback(unsigned int clientid, const boost::system::error_code& error)
{
	memset(gPool->clientPool.find(clientid)->second.buffer, NULL, sizeof(gPool->clientPool.find(clientid)->second.buffer));

	if(!error) // OK
	{
		gPool->clientPool.find(clientid)->second.sockid->async_receive(boost::asio::buffer(gPool->clientPool.find(clientid)->second.buffer, sizeof(gPool->clientPool.find(clientid)->second.buffer)), boost::bind(&amxSocket::asyncReadCallback, clientid, boost::asio::placeholders::error, boost::asio::placeholders::bytes_transferred));
	}
	else // lost connection
	{
		
	}
}



std::string amxSocket::asyncRead(unsigned int clientid)
{
	return gPool->clientPool.find(clientid)->second.buffer;
}



void amxSocket::asyncReadCallback(unsigned int clientid, const boost::system::error_code& error, std::size_t transf)
{
	if(!error)
	{
		gDebug->Log("Got '%s' (%i bytes) from client %i", gPool->clientPool.find(clientid)->second.buffer, transf, clientid);
	}
	else // lost connection
	{

	}
}