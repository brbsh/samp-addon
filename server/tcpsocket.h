#pragma once



#include "server.h"





class amxSocket
{

public:

	amxSocket(std::string ip, unsigned int port, unsigned int maxclients);
	virtual ~amxSocket();

	boost::thread *getThreadInstance() const
	{
		return this->acceptThread.get();
	}

	boost::mutex *getMutexInstance() const
	{
		return this->acceptMutex.get();
	}

	bool IsClientConnected(unsigned int clientid);
	void KickClient(unsigned int clientid);

	void asyncWrite(unsigned int clientid, std::string data);
	std::string asyncRead(unsigned int clientid);

	static void asyncWriteCallback(unsigned int clientid, const boost::system::error_code& error);
	static void asyncReadCallback(unsigned int clientid, const boost::system::error_code& error, std::size_t transf);

	static void AcceptThread(std::string ip, unsigned int port, unsigned int maxClients);

private:

	boost::shared_ptr<boost::thread> acceptThread;
	boost::shared_ptr<boost::mutex> acceptMutex;
};