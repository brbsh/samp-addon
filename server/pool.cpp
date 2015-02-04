#pragma once



#include "pool.h"





boost::shared_ptr<amxPool> gPool;


extern amxDebug *gDebug;





amxPool::amxPool()
{
	gDebug->Log("Pool constructor called");

	this->pluginState = false;

	this->clientPool.clear();
	this->serverPool.clear();
}



amxPool::~amxPool()
{
	gDebug->Log("Pool destructor called");

	this->pluginState = false;
}



void amxPool::setPluginStatus(bool status)
{
	boost::unique_lock<boost::shared_mutex> lockit(this->pstMutex);
	this->pluginState = status;
}



bool amxPool::getPluginStatus()
{
	boost::shared_lock<boost::shared_mutex> lockit(this->pstMutex);
	return this->pluginState;
}



void amxPool::setServerVar(std::string key, std::string value)
{
	boost::unique_lock<boost::shared_mutex> lockit(this->spMutex);
	this->serverPool[key] = value;
}



std::string amxPool::getServerVar(std::string key)
{
	boost::shared_lock<boost::shared_mutex> lockit(this->spMutex);
	return this->serverPool.find(key)->second;
}



void amxPool::resetOwnPool(unsigned int clientid)
{
	boost::unique_lock<boost::shared_mutex> lockit(this->cpMutex);
	this->clientPool.erase(clientid);
}



bool amxPool::hasOwnPool(unsigned int clientid)
{
	boost::shared_lock<boost::shared_mutex> lockit(this->cpMutex);
	return !(!this->clientPool.count(clientid));
}



void amxPool::setClientPool(unsigned int clientid, amxPool::clientPoolS struc)
{
	boost::unique_lock<boost::shared_mutex> lockit(this->cpMutex);
	this->clientPool[clientid] = struc;
}



amxPool::clientPoolS amxPool::getClientPool(unsigned int clientid)
{
	boost::shared_lock<boost::shared_mutex> lockit(this->cpMutex);
	return this->clientPool.find(clientid)->second;
}