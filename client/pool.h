#pragma once



#include "client.h"





class addonPool
{

public:

	addonPool();
	virtual ~addonPool();

	void setVar(std::string key, std::string value);
	std::string getVar(std::string key);

private:

	boost::shared_mutex mapMutex;
	boost::unordered_map<std::string, std::string> privPool;
};