#pragma once



#include "server.h"





class amxSocket
{

public:

	amxSocket(std::string ip, unsigned short port, unsigned int maxclients);
	virtual ~amxSocket();

	boost::thread *getThreadInstance() const
	{
		return threadInstance.get();
	}

	bool IsClientConnected(unsigned int clientid);
	void KickClient(unsigned int clientid);

	static void acceptThread(std::string ip, unsigned short port, unsigned int maxclients, unsigned short workerID);

private:

	boost::shared_ptr<boost::thread> threadInstance;
	//boost::shared_ptr<boost::thread_group> threadGroup;
};



class amxAsyncSession
{

public:

	amxAsyncSession(boost::asio::io_service& io_service) : poolHandle(io_service)
	{

	}

	amxPool::clientPoolS& pool()
	{
		return poolHandle;
	}

	void startSession(unsigned int binded_clid);
	void readHandle(unsigned int clientid, const char *buffer, const boost::system::error_code& error, std::size_t bytes_rx);
	void writeTo(unsigned int clientid, std::string data);
	void writeHandle(unsigned int clientid, const boost::system::error_code& error, std::size_t bytes_tx);

private:

	amxPool::clientPoolS poolHandle;
};



class amxAsyncServer
{

public:

	amxAsyncServer(boost::asio::io_service& io_service, std::string ip, unsigned short port, unsigned int maxclients) : io_s(io_service), acceptor(io_service, boost::asio::ip::tcp::endpoint(boost::asio::ip::address::from_string(ip), port))
	{
		asyncAcceptor(maxclients);
	}

	void asyncAcceptor(unsigned int maxclients);
	void asyncHandler(amxAsyncSession *new_session, const boost::system::error_code& error, unsigned int maxclients);

private:

	boost::asio::io_service& io_s;
	boost::asio::ip::tcp::acceptor acceptor;
};