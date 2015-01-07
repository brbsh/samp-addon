#pragma once



#include "pool.h"





boost::shared_ptr<amxPool> gPool;


extern boost::shared_ptr<amxDebug> gDebug;





amxPool::amxPool()
{
	gDebug->Log("Pool constructor called");

	this->pluginInit.store(false);

	//this->clientPool.clear();
	this->serverPool.clear();
}



amxPool::~amxPool()
{
	gDebug->Log("Pool destructor called");

	this->pluginInit.store(false);
}



void amxPool::setServerVar(std::string key, std::string value, boost::mutex *mutex)
{
	mutex->lock();
	this->serverPool[key] = value;
	mutex->unlock();
}



std::string amxPool::getServerVar(std::string key, boost::mutex *mutex)
{
	boost::mutex::scoped_lock lock(*mutex);

	return this->serverPool.find(key)->second;
}