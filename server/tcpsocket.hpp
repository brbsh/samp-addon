#pragma once



#include "server.hpp"





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
	void KickClient(unsigned int clientid, std::string reason);

	static void acceptThread(std::string ip, unsigned short port, unsigned int maxclients, unsigned short workerID);
	static void deadlineThread(unsigned int maxclients);

private:

	boost::shared_ptr<boost::thread> threadInstance;
	boost::shared_ptr<boost::thread> deadlineThreadInstance;
	//boost::shared_ptr<boost::thread_group> threadGroup;
};



class amxAsyncServer
{

public:

	amxAsyncServer(boost::asio::io_service& io_service, std::string ip, unsigned short port, unsigned int maxclients) : io_s(io_service), acceptor(io_service, boost::asio::ip::tcp::endpoint(boost::asio::ip::address::from_string(ip), port))
	{
		asyncAcceptor(maxclients);
	}

	~amxAsyncServer()
	{

	}

	void asyncAcceptor(unsigned int maxclients);
	void asyncHandler(amxAsyncSession *new_session, const boost::system::error_code& error, unsigned int maxclients);
	void sessionRemove(boost::asio::ip::address ip);

private:

	boost::asio::io_service& io_s;
	boost::asio::ip::tcp::acceptor acceptor;
	std::list<boost::asio::ip::address> connectedIPS;
	boost::mutex cIPMutex;
};



class amxAsyncSession
{

public:

	amxAsyncSession(boost::asio::io_service& io_service, amxAsyncServer *server) : poolHandle(io_service)
	{
		parentWorker = server;
	}

	~amxAsyncSession()
	{
		//poolHandle.sock.cancel();
		poolHandle.sock.shutdown(boost::asio::socket_base::shutdown_both);
		poolHandle.sock.close();
	}

	amxPool::clientPoolS& pool()
	{
		return poolHandle;
	}

	amxAsyncServer *worker() const
	{
		return parentWorker;
	}

	void startSession(unsigned int binded_clid);
	void readHandle(unsigned int clientid, const char *buffer, const boost::system::error_code& error, std::size_t bytes_rx);
	static void writeTo(unsigned int clientid, std::string data);
	void writeHandle(unsigned int clientid, const boost::system::error_code& error, std::size_t bytes_tx);

private:

	amxAsyncServer *parentWorker;
	amxPool::clientPoolS poolHandle;
};