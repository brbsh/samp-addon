#pragma once



#include "server.h"//#include "pool.h"





boost::shared_ptr<amxPool> gPool;


extern amxDebug *gDebug;





amxPool::amxPool()
{
	gDebug->Log("Pool constructor called");

	pluginState = false;

	clientPool.clear();
	serverPool.clear();
}



amxPool::~amxPool()
{
	gDebug->Log("Pool destructor called");

	pluginState = false;
}



void amxPool::setPluginStatus(bool status)
{
	boost::unique_lock<boost::shared_mutex> lockit(pstMutex);
	pluginState = status;
}



bool amxPool::getPluginStatus()
{
	boost::shared_lock<boost::shared_mutex> lockit(pstMutex);
	return pluginState;
}



void amxPool::setServerVar(std::string key, amxPool::svrData struc)
{
	boost::unique_lock<boost::shared_mutex> lockit(spMutex);
	serverPool[key] = struc;
}



amxPool::svrData amxPool::getServerVar(std::string key)
{
	boost::shared_lock<boost::shared_mutex> lockit(spMutex);
	return serverPool.find(key)->second;
}



void amxPool::resetOwnSession(unsigned int clientid)
{
	boost::unique_lock<boost::shared_mutex> lockit(cpMutex);

	amxAsyncSession *session = clientPool.find(clientid)->second;

	session->pool().sock.cancel();
	session->pool().sock.shutdown(boost::asio::socket_base::shutdown_both);
	session->pool().sock.close();

	gDebug->Log("Client %i disconnected", clientid);

	delete session;
	clientPool.erase(clientid);
}



bool amxPool::hasOwnSession(unsigned int clientid)
{
	boost::shared_lock<boost::shared_mutex> lockit(cpMutex);
	return !(!clientPool.count(clientid));
}



void amxPool::setClientSession(unsigned int clientid, amxAsyncSession *session)
{
	boost::unique_lock<boost::shared_mutex> lockit(cpMutex);
	clientPool[clientid] = session;
}



amxAsyncSession *amxPool::getClientSession(unsigned int clientid)
{
	boost::shared_lock<boost::shared_mutex> lockit(cpMutex);
	return clientPool.find(clientid)->second;
}