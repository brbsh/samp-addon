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
	{"samp_addon_init", n_addon_init},
	{NULL, NULL}
};



// native samp_addon_init(ip[], port, maxplayers);
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

	if((params[3] < 0) || (params[3] > 999))
	{
		logprintf("samp-addon: Invalid max players value passed to native 'samp_addon_init' (%i)", params[3]);

		return NULL;
	}

	if(gInit)
	{
		logprintf("samp-addon: Duplicate init interrupted");

		return NULL;
	}

	logprintf("\nsamp-addon: Init with address: %s:%i, max clients is %i\n", ip.c_str(), (params[2] + 1), params[3]);

	gInit = true;

	gPool->Set(-1, "ip", ip);
	gPool->Set(-1, "port", formatString() << (params[2] + 1));
	gPool->Set(-1, "maxclients", formatString() << params[3]);

	gThread->Start(process_thread, NULL);
	
	return 1;
}