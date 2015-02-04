#pragma once



#include "core.h"





boost::shared_ptr<amxCore> gCore;


extern amxDebug *gDebug;
extern boost::shared_ptr<amxPool> gPool;
extern boost::shared_ptr<amxSocket> gSocket;





amxCore::amxCore()
{
	gDebug->Log("Core constructor called");

	gPool = boost::shared_ptr<amxPool>(new amxPool());
	this->threadInstance = boost::shared_ptr<boost::thread>(new boost::thread(boost::bind(&amxCore::Thread)));
}



amxCore::~amxCore()
{
	gDebug->Log("Core destructor called");

	this->threadInstance->interruption_requested();
}



void amxCore::processFunc()
{

}



void amxCore::Thread()
{
	assert(gCore->getThreadInstance()->get_id() == boost::this_thread::get_id());

	gDebug->Log("Started Core thread with id 0x%x", gCore->getThreadInstance()->native_handle());

	while(!gPool->getPluginStatus())
		boost::this_thread::sleep_for(boost::chrono::seconds(1));

	std::string ip = gPool->getServerVar("serverIP");
	unsigned int port = atoi(gPool->getServerVar("serverPort").c_str());
	unsigned int maxclients = atoi(gPool->getServerVar("maxClients").c_str());

	gSocket = boost::shared_ptr<amxSocket>(new amxSocket(ip, port, maxclients));

	do
	{
		boost::this_thread::disable_interruption di;

		gCore->processFunc();

		boost::this_thread::restore_interruption re(di);
		boost::this_thread::sleep_for(boost::chrono::milliseconds(10));
	}
	while(gPool->getPluginStatus());
}