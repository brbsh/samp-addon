#pragma once



#include "server.h"//#include "tcpsocket.h"





boost::shared_ptr<amxSocket> gSocket;


extern amxDebug *gDebug;
extern boost::shared_ptr<amxPool> gPool;





amxSocket::amxSocket(std::string ip, unsigned short port, unsigned int maxclients)
{
	gDebug->Log("Socket constructor called");

	unsigned short worker_count = /*boost::thread::hardware_concurrency()*/ 1;

	threadInstance = boost::shared_ptr<boost::thread>(new boost::thread(boost::bind(&amxSocket::acceptThread, ip, port, maxclients, worker_count)));
	/*threadGroup = boost::shared_ptr<boost::thread_group>(new boost::thread_group);

	for(short i = 0; i < worker_count; i++)
		threadGroup->create_thread(boost::bind(&amxSocket::acceptThread, ip, port, maxclients, (i + 1)));*/
}



amxSocket::~amxSocket()
{
	gDebug->Log("Socket destructor called");

	threadInstance->interruption_requested();
	//threadGroup->interrupt_all();
}



bool amxSocket::IsClientConnected(unsigned int clientid)
{
	return gPool->hasOwnSession(clientid);
}



void amxSocket::KickClient(unsigned int clientid)
{
	if(!this->IsClientConnected(clientid))
		return;

	amxAsyncSession *session = gPool->getClientSession(clientid);
	gPool->resetOwnSession(clientid);

	gDebug->Log("Client %i was kicked", clientid);
}



void amxSocket::acceptThread(std::string ip, unsigned short port, unsigned int maxclients, unsigned short workerID)
{
	gDebug->Log("Thread amxSocket::acceptThread(worker #%i) successfuly started", workerID);

	boost::asio::io_service io_service;

	while(gPool->getPluginStatus())
	{
		try
		{
			amxAsyncServer server(io_service, ip, port, maxclients);
			gDebug->Log("TCP worker #%i started on %s:%i with max connections: %i", workerID, ip.c_str(), port, maxclients);
			io_service.run();
		}
		catch(boost::system::system_error& err)
		{
			gDebug->Log("Error while processing TCP worker #%i execution (What: %s)", workerID, err.what());
		}

		gDebug->Log("Restarting TCP worker #%i due to stop detected...", workerID);
	}
}



void amxAsyncServer::asyncAcceptor(unsigned int maxclients)
{
	amxAsyncSession *new_session = new amxAsyncSession(io_s);
	acceptor.async_accept(new_session->pool().sock, boost::bind(&amxAsyncServer::asyncHandler, this, new_session, boost::asio::placeholders::error, maxclients));
}



void amxAsyncServer::asyncHandler(amxAsyncSession *new_session, const boost::system::error_code& error, unsigned int maxclients)
{
	if(!error)
	{
		unsigned int clientid = maxclients;

		for(unsigned int i = 0; i != maxclients; i++)
		{
			if(!gSocket->IsClientConnected(i))
			{
				clientid = i;

				break;
			}
		}

		if(clientid == maxclients)
		{
			gDebug->Log("Cannot accept connection from %s: Server is full", new_session->pool().sock.remote_endpoint().address().to_string().c_str());

			delete new_session;
			//server is full
		}

		new_session->startSession(clientid);
	}
	else
	{
		gDebug->Log("Incoming connection from %s failed (What: %s)", new_session->pool().sock.remote_endpoint().address().to_string().c_str(), error.message().c_str());

		delete new_session;
	}

	asyncAcceptor(maxclients);
}



void amxAsyncSession::startSession(unsigned int binded_clid)
{
	poolHandle.ip = poolHandle.sock.remote_endpoint().address().to_string();
	poolHandle.sID = NULL;

	gPool->setClientSession(binded_clid, this);
	gDebug->Log("Incoming connection from %s (binded clientid: %i)", poolHandle.ip.c_str(), binded_clid);

	poolHandle.sock.async_read_some(boost::asio::buffer(poolHandle.buffer, sizeof poolHandle.buffer), boost::bind(&amxAsyncSession::readHandle, this, binded_clid, poolHandle.buffer, boost::asio::placeholders::error, boost::asio::placeholders::bytes_transferred));
}



void amxAsyncSession::readHandle(unsigned int clientid, const char *buffer, const boost::system::error_code& error, std::size_t bytes_rx)
{
	if(!error)
	{
		poolHandle.pendingQueue.push(buffer);
		gDebug->Log("Got '%s' (length: %i) from client %i", buffer, bytes_rx, clientid);

		writeTo(clientid, "The quick brown fox jumps over the lazy dog");
		//poolHandle.sock.async_write_some(boost::asio::buffer("12345678900987654321", 20), boost::bind(&amxAsyncSession::writeHandle, this, clientid, boost::asio::placeholders::error, boost::asio::placeholders::bytes_transferred));
	}
	else
	{
		gDebug->Log("Cannot read data from client %i (What: %i %s)", clientid, error.value(), error.message().c_str());
		gPool->resetOwnSession(clientid);
	}
}



void amxAsyncSession::writeTo(unsigned int clientid, std::string data)
{
	if(!gSocket->IsClientConnected(clientid))
		return;

	amxAsyncSession *sessid = gPool->getClientSession(clientid);
	sessid->pool().sock.async_write_some(boost::asio::buffer(data, data.length()), boost::bind(&amxAsyncSession::writeHandle, sessid, clientid, boost::asio::placeholders::error, boost::asio::placeholders::bytes_transferred));
}



void amxAsyncSession::writeHandle(unsigned int clientid, const boost::system::error_code& error, std::size_t bytes_tx)
{
	if(!error)
	{
		memset(poolHandle.buffer, NULL, sizeof poolHandle.buffer);
		poolHandle.sock.async_read_some(boost::asio::buffer(poolHandle.buffer, sizeof poolHandle.buffer), boost::bind(&amxAsyncSession::readHandle, this, clientid, poolHandle.buffer, boost::asio::placeholders::error, boost::asio::placeholders::bytes_transferred));
	}
	else
	{
		gDebug->Log("Cannot send data to client %i (What: %i %s)", clientid, error.value(), error.message().c_str());
		gPool->resetOwnSession(clientid);
	}
}