#pragma once



#include "addon.h"





class amxPool
{

public:

	void Set(int index, std::string key, std::string value);
	std::string Get(int index, std::string key);

private:

	std::map<int, std::map<std::string, std::string>> playerPool;
	std::map<std::string, std::string> serverPool;
};