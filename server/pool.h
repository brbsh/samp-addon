#pragma once



#include "server.h"





class amxPool
{

public:

	static struct clientPoolS
	{
		boost::shared_ptr<boost::asio::ip::tcp::socket> sockid;

		long int sID;
		std::string ip;

		char buffer[8192];
	};

	boost::atomic<bool> pluginInit;
	boost::unordered_map<unsigned int, amxPool::clientPoolS> clientPool;
	boost::unordered_map<std::string, std::string> serverPool;

	amxPool();
	virtual ~amxPool();

	void setServerVar(std::string key, std::string value, boost::mutex *mutex);
	std::string getServerVar(std::string key, boost::mutex *mutex);
};