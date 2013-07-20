#pragma once



#include "addon.h"





struct transfer
{
	bool Active;

	std::string file;
};



struct client
{
	bool Auth;
	int Serial;
};



struct cliPool
{
	boost::shared_ptr<boost::asio::ip::tcp::socket> socketid;
	std::string ip;

	struct transfer Transfer;
	struct client Client;
};



class amxPool
{

public:

	amxPool();
	~amxPool();

	bool pluginInit;

	boost::unordered_map<int, cliPool> clientPool;
};