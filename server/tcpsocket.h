#pragma once



#ifndef TCPSOCKET_H
#define TCPSOCKET_H

#include "server.h"





class amxSocket
{

public:

	amxSocket(std::string ip, unsigned int port, unsigned int maxclients);
	virtual ~amxSocket();

	boost::thread *getThreadInstance() const
	{
		return this->threadInstance.get();
	}

	bool IsClientConnected(unsigned int clientid);
	void KickClient(unsigned int clientid);

	static void acceptThread(std::string ip, unsigned short port, unsigned int maxClients);

private:

	boost::shared_ptr<boost::thread> threadInstance;
};



class amxAsyncSession
{

public:

	amxAsyncSession(boost::asio::io_service& io_service) : socketHandle(io_service)
	{

	}

	boost::asio::ip::tcp::socket& socket()
	{
		return socketHandle;
	}

	void startSession(unsigned int binded_clid);
	void readHandle(unsigned int clientid, const char *buffer, const boost::system::error_code& error, std::size_t bytes_trx);
	void writeHandle(unsigned int clientid, const boost::system::error_code& error);

private:

	unsigned int clientid;
	boost::asio::ip::tcp::socket socketHandle;
};



class amxAsyncServer
{

public:

	amxAsyncServer(boost::asio::io_service& io_service, std::string ip, unsigned short port, unsigned int maxclients) 
		: maxcl(maxclients), io_s(io_service), acceptor(io_service, boost::asio::ip::tcp::endpoint(boost::asio::ip::address::from_string(ip), port))
	{
		asyncAcceptor();
	}

	void asyncAcceptor();
	void asyncHandler(amxAsyncSession *new_session, const boost::system::error_code& error);

private:

	unsigned int& maxcl;
	boost::asio::io_service& io_s;
	boost::asio::ip::tcp::acceptor acceptor;
};





#endif