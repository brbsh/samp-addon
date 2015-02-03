#pragma once



#include "pool.h"





boost::shared_ptr<addonPool> gPool;


extern boost::shared_ptr<addonDebug> gDebug;





addonPool::addonPool()
{
	gDebug->traceLastFunction("addonPool::addonPool() at 0x?????");
	gDebug->Log("Pool constructor called");

	this->privPool.clear();
}



addonPool::~addonPool()
{
	gDebug->traceLastFunction("addonPool::~addonPool() at 0x?????");
	gDebug->Log("Pool destructor called");
}



void addonPool::setVar(std::string key, std::string value)
{
	boost::unique_lock<boost::shared_mutex> lockit(this->mapMutex);

	this->privPool[key] = value;

	lockit.unlock();
}



std::string addonPool::getVar(std::string key)
{
	boost::shared_lock<boost::shared_mutex> lockit(this->mapMutex);

	return this->privPool.find(key)->second;
}