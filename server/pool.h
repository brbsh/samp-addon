#pragma once



#include "addon.h"





struct sockPool
{
	int socketid;
	bool transfer;

	std::string ip;
};



struct cliPool
{
	bool Active;
	int serial;
	bool auth;
	int in_menu;
	float gravity;
	float gamespeed;
};



struct transPool
{
	bool send;
	long file_length;
	
	std::string file;
	std::string remote_file;
};



class amxPool
{

public:

	amxPool();
	~amxPool();

	bool pluginInit;

	boost::unordered_map<int, sockPool> socketPool;
	boost::unordered_map<int, cliPool> clientPool;
	boost::unordered_map<int, transPool> transferPool;
};