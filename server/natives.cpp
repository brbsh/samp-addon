#pragma once



#include "natives.h"



//extern amxMutex *gMutex;
extern amxPool *gPool;
extern amxSocket *gSocket;
extern amxString *gString;
//extern amxThread *gThread;
extern logprintf_t logprintf;

extern std::queue<amxConnectError> amxConnectErrorQueue;





const AMX_NATIVE_INFO amxNatives::addonNatives[] =
{
	{"InitAddon", amxNatives::Init},
	{"IsClientConnected", amxNatives::IsClientConnected},
	{"KickClient", amxNatives::KickClient},
	{"GetClientSerial", amxNatives::GetClientSerial},
	{"GetClientScreenshot", amxNatives::GetClientScreenshot},
	{"TransferLocalFile", amxNatives::TransferLocalFile},
	{"TransferRemoteFile", amxNatives::TransferRemoteFile},

	{NULL, NULL}
};



// native InitAddon(ip[], port, maxplayers);
cell AMX_NATIVE_CALL amxNatives::Init(AMX *amx, cell *params)
{
	if(!arguments(3))
	{
		logprintf("samp-addon: Invalid argument count in native 'samp_addon_init' (%i)", (params[0] >> 4));

		return NULL;
	}

	std::string ip = gString->Get(amx, params[1]);

	if(!ip.length() || (ip.length() > 15))
	{
		logprintf("samp-addon: NULL ip argument passed to native 'samp_addon_init'");

		return NULL;
	}

	if((params[2] < 1024) || (params[2] > 65535))
	{
		logprintf("samp-addon: Invalid port value passed to native 'samp_addon_init' (%i)", params[2]);

		return NULL;
	}

	if((params[3] < 0) || (params[3] > 1000))
	{
		logprintf("samp-addon: Invalid max players value passed to native 'samp_addon_init' (%i)", params[3]);

		return NULL;
	}

	if(gPool->pluginInit)
	{
		logprintf("samp-addon: Duplicate init interrupted");

		return NULL;
	}

	logprintf("\nsamp-addon: Init with address: %s:%i, max clients is %i\n", ip.c_str(), (params[2] + 1), params[3]);

	boost::mutex::scoped_lock lock(gPool->Mutex);
	gPool->pluginInit = true;
	lock.unlock();

	gSocket->MaxClients(params[3]);
	gSocket->Bind(ip);
	gSocket->Listen((params[2] + 1));

	boost::thread process(boost::bind(&amxProcess::Thread, amx));
	
	return 1;
}



// native IsClientConnected(clientid)
cell AMX_NATIVE_CALL amxNatives::IsClientConnected(AMX *amx, cell *params)
{
	if(!arguments(1))
	{
		logprintf("samp-addon: Invalid argument count in native 'IsClientConnected' (%i)", (params[0] >> 2));

		return NULL;
	}

	return (gSocket->IsClientConnected(params[1])) ? 1 : 0;
}



// native KickClient(clientid)
cell AMX_NATIVE_CALL amxNatives::KickClient(AMX *amx, cell *params)
{
	if(!arguments(1))
	{
		logprintf("samp-addon: Invalid argument count in native 'KickClient' (%i)", (params[0] >> 2));

		return NULL;
	}

	gSocket->KickClient(params[1]);

	return 1;
}



// native GetClientSerial(clientid);
cell AMX_NATIVE_CALL amxNatives::GetClientSerial(AMX *amx, cell *params)
{
	if(!arguments(1))
	{
		logprintf("samp-addon: Invalid argument count in native 'GetClientVolumeID' (%i)", (params[0] >> 2));

		return NULL;
	}

	if(!gSocket->IsClientConnected(params[1]))
		return NULL;

	int ret = gPool->clientPool[params[1]].serial;

	return ret;
}



// native GetClientScreenshot(clientid, remote_filename[]);
cell AMX_NATIVE_CALL amxNatives::GetClientScreenshot(AMX *amx, cell *params)
{
	if(!arguments(2))
	{
		logprintf("samp-addon: Invalid argument count in native 'TakePlayerScreenshot' (%i)", (params[0] >> 2));

		return NULL;
	}

	if(!gSocket->IsClientConnected(params[1]))
		return NULL;

	std::string filename = gString->Get(amx, params[2]);

	if(!filename.length() || (filename.length() > 256) || (filename.find(".png") == std::string::npos))
	{
		logprintf("samp-addon: Invalid file name format");

		return NULL;
	}

	gSocket->Send(params[1], formatString() << "TCPQUERY SERVER_CALL" << " " << 1003 << " " << filename);

	return 1;
}



// native TransferLocalFile(file[], toclient, remote_name[]);
cell AMX_NATIVE_CALL amxNatives::TransferLocalFile(AMX *amx, cell *params)
{
	if(!arguments(3))
		return NULL;

	if(!gSocket->IsClientConnected(params[2]))
		return NULL;

	transPool pool;
	sockPool sPool;

	pool.send = true;
	pool.file = gString->Get(amx, params[1]);
	pool.remote_file = gString->Get(amx, params[3]);

	boost::mutex::scoped_lock lock(gPool->Mutex);
	sPool = gPool->socketPool[params[2]];

	sPool.transfer = true;

	gPool->transferPool[params[2]] = pool;
	gPool->socketPool[params[2]] = sPool;
	lock.unlock();

	boost::thread send(boost::bind(&amxTransfer::SendThread, (int)params[2]));

	return 1;
}



// native TransferRemoteFile(remote_file[], fromclient, file[]);
cell AMX_NATIVE_CALL amxNatives::TransferRemoteFile(AMX *amx, cell *params)
{
	if(!arguments(3))
		return NULL;

	if(!gSocket->IsClientConnected(params[2]))
		return NULL;

	transPool pool;

	pool.send = false;
	pool.file = gString->Get(amx, params[3]);
	pool.remote_file = gString->Get(amx, params[1]);

	boost::mutex::scoped_lock lock(gPool->Mutex);
	gPool->transferPool[params[2]] = pool;
	lock.unlock();

	gSocket->Send(params[2], formatString() << "TCPQUERY SERVER_CALL" << " " << 2001 << " " << pool.remote_file);

	return 1;
}