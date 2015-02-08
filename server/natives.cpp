#pragma once



#include "server.hpp"





extern logprintf_t logprintf;

extern boost::shared_ptr<amxPool> gPool;
extern boost::shared_ptr<amxSocket> gSocket;





const AMX_NATIVE_INFO amxNatives::addonNatives[] =
{
	{"Addon_Init", amxNatives::InitAddon},
	{"Addon_IsClientConnected", amxNatives::IsClientConnected},
	{"Addon_KickClient", amxNatives::KickClient},
	{"Addon_GetClientSerial", amxNatives::GetClientSerial},
	//{"Addon_GetClientScreenshot", amxNatives::GetClientScreenshot},
	//{"TransferLocalFile", amxNatives::TransferLocalFile},
	//{"TransferRemoteFile", amxNatives::TransferRemoteFile},

	{NULL, NULL}
};



// native Addon_Init(ip[], port = 7777, maxplayers = MAX_PLAYERS);
cell AMX_NATIVE_CALL amxNatives::InitAddon(AMX *amx, cell *params)
{
	if(!arguments(3))
	{
		logprintf("SAMP-Addon: Invalid argument count in native 'InitAddon' (%i)", (params[0] >> 4));

		return NULL;
	}

	std::string ip = amxString::Get(amx, params[1]);
	int port = params[2];
	int maxplayers = params[3];

	if(!ip.length() || (ip.length() > 15))
	{
		logprintf("SAMP-Addon: NULL ip argument passed to native 'InitAddon'");

		return NULL;
	}

	if((port < 1024) || (port > 65535))
	{
		logprintf("SAMP-Addon: Invalid port value passed to native 'InitAddon' (%i)", port);

		return NULL;
	}

	if((maxplayers < 0) || (maxplayers > 1000))
	{
		logprintf("SAMP-Addon: Invalid max players value passed to native 'InitAddon' (%i)", maxplayers);

		return NULL;
	}

	if(gPool->getPluginStatus())
	{
		logprintf("SAMP-Addon: Duplicate init interrupted");

		return NULL;
	}

	logprintf("\nSAMP-Addon: Requested init with address: %s:%i, max clients is %i\n", ip.c_str(), ++port, maxplayers);

	amxPool::svrData push;

	push.reset();
	push.string = ip;
	gPool->setServerVar("ip", push);

	push.reset();
	push.integer = port;
	gPool->setServerVar("port", push);

	push.reset();
	push.integer = maxplayers;
	gPool->setServerVar("maxclients", push);

	gPool->setPluginStatus(true);
	
	return 1;
}



// native Addon_IsClientConnected(clientid);
cell AMX_NATIVE_CALL amxNatives::IsClientConnected(AMX *amx, cell *params)
{
	if(!arguments(1))
	{
		logprintf("SAMP-Addon: Invalid argument count in native 'IsClientConnected' (%i)", (params[0] >> 2));

		return NULL;
	}

	return (gSocket->IsClientConnected(params[1])) ? 1 : 0;
}



// native Addon_KickClient(clientid);
cell AMX_NATIVE_CALL amxNatives::KickClient(AMX *amx, cell *params)
{
	if(!arguments(1))
	{
		logprintf("SAMP-Addon: Invalid argument count in native 'KickClient' (%i)", (params[0] >> 2));

		return NULL;
	}

	gSocket->KickClient(params[1]);

	return 1;
}



// native Addon_GetClientSerial(clientid);
cell AMX_NATIVE_CALL amxNatives::GetClientSerial(AMX *amx, cell *params)
{
	if(!arguments(1))
	{
		logprintf("samp-addon: Invalid argument count in native 'GetClientSerial' (%i)", (params[0] >> 2));

		return NULL;
	}

	int clientid = params[1];

	if(!gSocket->IsClientConnected(clientid))
		return NULL;

	amxAsyncSession *session = gPool->getClientSession(clientid);

	return session->pool().sID;
}



// native Addon_GetClientScreenshot(clientid, remote_filename[]);
/*cell AMX_NATIVE_CALL amxNatives::GetClientScreenshot(AMX *amx, cell *params)
{
	if(!arguments(2))
	{
		logprintf("samp-addon: Invalid argument count in native 'TakePlayerScreenshot' (%i)", (params[0] >> 2));

		return NULL;
	}

	int clientid = params[1];

	if(!gSocket->IsClientConnected(clientid))
		return NULL;

	std::string filename = amxString::Get(amx, params[2]);

	if(!filename.length() || (filename.length() > 256) || (filename.find(".png") != (filename.length() - 4)))
	{
		logprintf("SAMP-Addon: Invalid file name format");

		return NULL;
	}

	gSocket->Send(clientid, strFormat() << "TCPQUERY SERVER_CALL" << " " << 1003 << " " << filename);

	return 1;
}



// native TransferLocalFile(file[], toclient, remote_filename[]);
cell AMX_NATIVE_CALL amxNatives::TransferLocalFile(AMX *amx, cell *params)
{
	if(!arguments(3))
		return NULL;

	int clientid = params[2];

	if(!gSocket->IsClientConnected(clientid))
		return NULL;

	cliPool cPool;

	boost::mutex::scoped_lock lock(gMutex);
	cPool = gPool->clientPool.find(clientid)->second;

	cPool.Transfer.Active = true;

	gPool->clientPool[clientid] = cPool;
	lock.unlock();

	boost::thread send(boost::bind(&amxTransfer::SendThread, clientid, gString->Get(amx, params[1]), gString->Get(amx, params[3])));

	return 1;
}



// native TransferRemoteFile(remote_filename[], fromclient, file[]);
cell AMX_NATIVE_CALL amxNatives::TransferRemoteFile(AMX *amx, cell *params)
{
	if(!arguments(3))
		return NULL;

	int clientid = params[2];

	if(!gSocket->IsClientConnected(clientid))
		return NULL;

	cliPool cPool;

	boost::mutex::scoped_lock lock(gMutex);
	cPool = gPool->clientPool.find(clientid)->second;

	cPool.Transfer.file = gString->Get(amx, params[3]);

	gPool->clientPool[clientid] = cPool;
	lock.unlock();

	gSocket->Send(clientid, formatString() << "TCPQUERY SERVER_CALL" << " " << 2001 << " " << gString->Get(amx, params[1]));

	return 1;
}*/