#pragma once



#include "pool.h"



amxPool *gPool;





void amxPool::Set(int index, std::string key, std::string value)
{
	if(index != -1)
	{
		std::map<std::string, std::string> ptr;

		ptr = this->playerPool.find(index)->second;
		ptr[key] = value;

		this->playerPool.insert(std::make_pair(index, ptr));
	}
	else
	{
		this->serverPool.insert(std::make_pair(key, value));
	}
}



std::string amxPool::Get(int index, std::string key)
{
	if(index != -1)
		return this->playerPool.find(index)->second.find(key)->second;
	else
		return this->serverPool.find(key)->second;
}
