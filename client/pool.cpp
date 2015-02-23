#pragma once



#include "client.hpp"





boost::shared_ptr<addonPool> gPool;


extern boost::shared_ptr<addonDebug> gDebug;





addonPool::addonPool()
{
	gDebug->traceLastFunction("addonPool::addonPool() at 0x?????");
	gDebug->Log("Pool constructor called");

	privPool.clear();
}



addonPool::~addonPool()
{
	gDebug->traceLastFunction("addonPool::~addonPool() at 0x?????");
	gDebug->Log("Pool destructor called");
}



void addonPool::setVar(std::string key, std::string value)
{
	boost::unique_lock<boost::shared_mutex> lockit(mapMutex);
	privPool[key] = value;
}



std::string addonPool::getVar(std::string key)
{
	boost::shared_lock<boost::shared_mutex> lockit(mapMutex);
	return privPool.find(key)->second;
}