#pragma once



#include "natives.h"



extern amxPool *gPool;
extern amxSocket *gSocket;
extern amxString *gString;
extern amxThread *gThread;
extern logprintf_t logprintf;
extern bool gInit;





AMX_NATIVE_INFO addonNatives[] =
{
	{"InitAddon", n_addon_init},
	{"IsClientConnected", n_addon_is_client_connected},
	{"KickClient", n_addon_kick_client},

	{"GetClientSerial", n_addon_get_serial},
	{"GetClientScreenshot", n_addon_get_screenshot},
	{NULL, NULL}
};



// native InitAddon(ip[], port, maxplayers);
cell AMX_NATIVE_CALL n_addon_init(AMX *amx, cell *params)
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

	gPool->pluginInit = true;

	gSocket->MaxClients(params[3]);
	gSocket->Bind(ip);
	gSocket->Listen((params[2] + 1));

	gThread->Start(process_thread, NULL);
	
	return 1;
}



// native IsClientConnected(clientid)
cell AMX_NATIVE_CALL n_addon_is_client_connected(AMX *amx, cell *params)
{
	if(!arguments(1))
	{
		logprintf("samp-addon: Invalid argument count in native 'IsClientConnected' (%i)", (params[0] >> 2));

		return NULL;
	}

	return (gSocket->IsClientConnected(params[1])) ? 1 : 0;
}



// native KickClient(clientid)
cell AMX_NATIVE_CALL n_addon_kick_client(AMX *amx, cell *params)
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
cell AMX_NATIVE_CALL n_addon_get_serial(AMX *amx, cell *params)
{
	if(!arguments(1))
	{
		logprintf("samp-addon: Invalid argument count in native 'GetClientVolumeID' (%i)", (params[0] >> 2));

		return NULL;
	}

	if(!gSocket->IsClientConnected(params[1]))
		return NULL;

	return gPool->clientPool[params[1]].serial;
}



// native GetClientScreenshot(clientid, remote_filename[]);
cell AMX_NATIVE_CALL n_addon_get_screenshot(AMX *amx, cell *params)
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

	gSocket->Send(params[1], formatString() << "TCPQUERY" << " " << "SERVER_CALL" << " " << 1003 << " " << filename);

	return 1;
}