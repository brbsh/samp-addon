#pragma once



#include "addon.h"





struct sockPool
{
	int socketid;
	std::string ip;
};



struct cliPool
{
	int serial;
	bool auth;
	int in_menu;
	float gravity;
	float gamespeed;
};



class amxPool
{

public:
	
	bool pluginInit;
	std::map<int, sockPool> socketPool;
	std::map<int, cliPool> clientPool;

	amxPool();
	~amxPool();
};