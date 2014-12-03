#pragma once



#include "server.h"





class amxPool
{

public:

	bool pluginInit;

	amxPool();
	virtual ~amxPool();

	void setServerVar(std::string key, std::string value, boost::mutex *mutex);
	std::string getServerVar(std::string key, boost::mutex *mutex);

private:

	//boost::unordered_map<int, cliPool> clientPool;
	boost::unordered_map<std::string, std::string> serverPool;
};