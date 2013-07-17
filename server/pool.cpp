#pragma once



#include "pool.h"





amxPool *gPool;





amxPool::amxPool()
{
	addonDebug("Pool constructor called");

	this->pluginInit = false;

	this->socketPool.clear();
	this->clientPool.clear();
	this->transferPool.clear();
}



amxPool::~amxPool()
{
	addonDebug("Pool deconstructor called");
}