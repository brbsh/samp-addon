#pragma once



#include "pool.h"





amxPool *gPool;





amxPool::amxPool()
{
	addonDebug("Pool constructor called");

	this->pluginInit = false;

	this->clientPool.clear();
}



amxPool::~amxPool()
{
	addonDebug("Pool deconstructor called");
}