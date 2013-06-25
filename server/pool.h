#pragma once



#include "addon.h"





struct sockPool
{
	int socketid;
	std::string ip;
	bool transfer;
};



struct cliPool
{
	int serial;
	bool auth;
	int in_menu;
	float gravity;
	float gamespeed;
};



struct transPool
{
	bool send;
	std::string file;
	std::string remote_file;
	long file_length;
};



class amxPool
{

public:
	
	bool pluginInit;

	boost::mutex Mutex;

	boost::unordered_map<int, sockPool> socketPool;
	boost::unordered_map<int, cliPool> clientPool;
	boost::unordered_map<int, transPool> transferPool;

	amxPool();
	~amxPool();
};