#pragma once



#include "pool.h"



amxPool *gPool;





amxPool::amxPool()
{
	this->pluginInit = false;

	this->socketPool.clear();
	this->clientPool.clear();
}