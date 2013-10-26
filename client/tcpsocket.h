#pragma once



#include "client.h"





class addonSocket
{

public:

	addonSocket();
	virtual ~addonSocket();

	boost::asio::io_service IOService;
};