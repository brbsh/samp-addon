#pragma once



#include "server.h"





class amxPool
{

public:

	static struct clientPoolS
	{
		clientPoolS(boost::asio::io_service& io_service) : sock(io_service)
		{

		}

		boost::asio::ip::tcp::socket sock;

		std::queue<std::string> pendingQueue;
		boost::shared_ptr<boost::mutex> pqMutex;

		long int sID;
		std::string ip;
		char buffer[2048];
	};

	static struct svrData
	{
		long integer;
		double floating;
		std::string string;

		void reset()
		{
			integer = 0;
			floating = 0.0f;
			string.clear();
		}
	};

	amxPool();
	virtual ~amxPool();

	void setPluginStatus(bool status);
	bool getPluginStatus();

	void setServerVar(std::string key, svrData struc);
	svrData getServerVar(std::string key);

	void resetOwnSession(unsigned int clientid);
	bool hasOwnSession(unsigned int clientid);
	void setClientSession(unsigned int clientid, amxAsyncSession *session);
	amxAsyncSession *getClientSession(unsigned int clientid);

private:

	bool pluginState;
	boost::shared_mutex pstMutex;

	boost::unordered_map<std::string, svrData> serverPool;
	boost::shared_mutex spMutex;

	boost::unordered_map<unsigned int, amxAsyncSession *> clientPool;
	boost::shared_mutex cpMutex;
};