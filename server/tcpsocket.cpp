#pragma once



#include "tcpsocket.h"





boost::shared_ptr<amxSocket> gSocket;


extern amxDebug *gDebug;
extern boost::shared_ptr<amxPool> gPool;





amxSocket::amxSocket(std::string ip, unsigned int port, unsigned int maxclients)
{
	gDebug->Log("Socket constructor called");

	this->threadInstance = boost::shared_ptr<boost::thread>(new boost::thread(boost::bind(&amxSocket::acceptThread, ip, port, maxclients)));
}



amxSocket::~amxSocket()
{
	gDebug->Log("Socket destructor called");

	this->threadInstance->interruption_requested();
}



bool amxSocket::IsClientConnected(unsigned int clientid)
{
	return gPool->hasOwnPool(clientid);
}



void amxSocket::KickClient(unsigned int clientid)
{
	if(!this->IsClientConnected(clientid))
		return;

	amxPool::clientPoolS struc = gPool->getClientPool(clientid);
/*	struc.sockid.cancel();
	struc.sockid.shutdown(boost::asio::socket_base::shutdown_both);
	struc.sockid.close();*/

	gPool->resetOwnPool(clientid);

	gDebug->Log("Client %i was kicked", clientid);
}



void amxSocket::acceptThread(std::string ip, unsigned short port, unsigned int maxclients)
{
	gDebug->Log("Thread amxSocket::Thread() successfuly started");

	boost::asio::io_service io_service;

	try
	{
		amxAsyncServer server(io_service, ip, port, maxclients);
		gDebug->Log("TCP server started on %s:%i with max connections: %i", ip.c_str(), port, maxclients);
		io_service.run();
	}
	catch(boost::system::system_error &err)
	{
		gDebug->Log("Error while processing TCP server execution (What: %s)", err.what());

		return;
	}
}



void amxAsyncServer::asyncAcceptor()
{
	amxAsyncSession *new_session = new amxAsyncSession(io_s);
	acceptor.async_accept(new_session->socket(), boost::bind(&amxAsyncServer::asyncHandler, this, new_session, boost::asio::placeholders::error));
}



void amxAsyncServer::asyncHandler(amxAsyncSession *new_session, const boost::system::error_code& error)
{
	if(!error)
	{
		int clientid = this->maxcl;

		for(unsigned int i = 0; i != this->maxcl; i++)
		{
			if(!gSocket->IsClientConnected(i))
			{
				clientid = i;

				break;
			}
		}

		if(clientid == this->maxcl)
		{
			//server is full
		}

		amxPool::clientPoolS push;

		push.ip = new_session->socket().remote_endpoint().address().to_string();
		//push.sockid = new_session->socket();
		push.sID = NULL;

		gDebug->Log("Incoming connection from %s (binded clientid: %i)", push.ip.c_str(), clientid);

		new_session->startSession(clientid);
	}
	else
	{
		gDebug->Log("Incoming connection from %s failed", new_session->socket().remote_endpoint().address().to_string().c_str());

		delete new_session;
	}

	this->asyncAcceptor();
}



void amxAsyncSession::startSession(unsigned int binded_clid)
{
	char buff[1024];

	this->clientid = binded_clid;
	this->socketHandle.async_read_some(boost::asio::buffer(buff, sizeof buff), boost::bind(&amxAsyncSession::readHandle, this, binded_clid, buff, boost::asio::placeholders::error, boost::asio::placeholders::bytes_transferred));
}



void amxAsyncSession::readHandle(unsigned int clientid, const char *buffer, const boost::system::error_code& error, std::size_t bytes_trx)
{
	if(!error)
	{
		gDebug->Log("Got '%s' (length: %i) from client %i", buffer, bytes_trx, clientid);

		this->socketHandle.async_write_some(boost::asio::buffer("12345678900987654321", 20), boost::bind(&amxAsyncSession::writeHandle, this, clientid, boost::asio::placeholders::error));
	}
	else
	{
		gDebug->Log("Cannot read data from client %i (What: %i %s)", clientid, error.value(), error.message().c_str());

		this->socketHandle.cancel();
		this->socketHandle.shutdown(boost::asio::socket_base::shutdown_both);
		this->socketHandle.close();

		delete this;
	}
}



void amxAsyncSession::writeHandle(unsigned int clientid, const boost::system::error_code& error)
{
	if(!error)
	{
		char buff[1024];

		this->socketHandle.async_read_some(boost::asio::buffer(buff, sizeof buff), boost::bind(&amxAsyncSession::readHandle, this, clientid, buff, boost::asio::placeholders::error, boost::asio::placeholders::bytes_transferred));
	}
	else
	{
		gDebug->Log("Cannot send data to client %i (What: %i %s)", clientid, error.value(), error.message().c_str());

		this->socketHandle.cancel();
		this->socketHandle.shutdown(boost::asio::socket_base::shutdown_both);
		this->socketHandle.close();

		delete this;
	}
}