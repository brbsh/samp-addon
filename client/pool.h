#pragma once



#include "client.h"





class addonPool
{

public:

	addonPool();
	virtual ~addonPool();

	void setVar(std::string key, std::string value, boost::mutex *mutex);
	std::string getVar(std::string key, boost::mutex *mutex);

private:

	boost::unordered_map<std::string, std::string> privPool;
};