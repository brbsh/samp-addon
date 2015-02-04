#pragma once



#ifndef POOL_H
#define POOL_H

#include "server.h"





class amxPool
{

public:

	static struct clientPoolS
	{
		//boost::asio::ip::tcp::socket sockid;

		std::queue<std::pair<unsigned int, std::string>> pendingQueue;
		std::queue<std::pair<unsigned int, std::string>> outputQueue;

		boost::shared_ptr<boost::mutex> sockMutex;
		boost::shared_ptr<boost::mutex> pqMutex;
		boost::shared_ptr<boost::mutex> oqMutex;

		long int sID;
		std::string ip;
	};

	amxPool();
	virtual ~amxPool();

	void setPluginStatus(bool status);
	bool getPluginStatus();

	void setServerVar(std::string key, std::string value);
	std::string getServerVar(std::string key);

	void resetOwnPool(unsigned int clientid);
	bool hasOwnPool(unsigned int clientid);
	void setClientPool(unsigned int clientid, amxPool::clientPoolS struc);
	amxPool::clientPoolS getClientPool(unsigned int clientid);

private:

	bool pluginState;
	boost::shared_mutex pstMutex;

	boost::unordered_map<std::string, std::string> serverPool;
	boost::shared_mutex spMutex;
	boost::unordered_map<unsigned int, amxPool::clientPoolS> clientPool;
	boost::shared_mutex cpMutex;
};





#endif