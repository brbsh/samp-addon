#pragma once



#include "server.hpp"





boost::shared_ptr<amxCore> gCore;


extern amxDebug *gDebug;
extern boost::shared_ptr<amxPool> gPool;
extern boost::shared_ptr<amxSocket> gSocket;





amxCore::amxCore()
{
	gDebug->Log("Core constructor called");

	gPool = boost::shared_ptr<amxPool>(new amxPool());
	threadInstance = boost::shared_ptr<boost::thread>(new boost::thread(boost::bind(&amxCore::Thread)));
}



amxCore::~amxCore()
{
	gDebug->Log("Core destructor called");

	threadInstance->interruption_requested();
}



void amxCore::processFunc()
{

}



void amxCore::Thread()
{
	assert(gCore->getThreadInstance()->get_id() == boost::this_thread::get_id());

	gDebug->Log("Thread amxCore::Thread() successfuly started");

	while(!gPool->getPluginStatus())
		boost::this_thread::sleep_for(boost::chrono::seconds(1));

	amxPool::svrData data = gPool->getServerVar("ip");
	std::string ip = data.string;

	data.reset();
	data = gPool->getServerVar("port");
	unsigned short port = data.integer;

	data.reset();
	data = gPool->getServerVar("maxclients");
	unsigned int maxclients = data.integer;

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