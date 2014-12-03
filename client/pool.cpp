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



void addonPool::setVar(std::string key, std::string value, boost::mutex *mutex)
{
	mutex->lock();
	this->privPool[key] = value;
	mutex->unlock();
}



std::string addonPool::getVar(std::string key, boost::mutex *mutex)
{
	boost::mutex::scoped_lock lock(*mutex);

	return this->privPool.find(key)->second;
}