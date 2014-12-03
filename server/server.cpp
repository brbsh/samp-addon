#pragma once



#include "server.h"



extern void *pAMXFunctions;

extern boost::shared_ptr<amxCore> gCore;
extern boost::shared_ptr<amxDebug> gDebug;
extern boost::shared_ptr<amxPool> gPool;
extern boost::shared_ptr<amxSocket> gSocket;


logprintf_t logprintf;





PLUGIN_EXPORT unsigned int PLUGIN_CALL Supports() 
{
    return (SUPPORTS_VERSION | SUPPORTS_AMX_NATIVES | SUPPORTS_PROCESS_TICK);
}



PLUGIN_EXPORT bool PLUGIN_CALL Load(void **ppData) 
{
    pAMXFunctions = ppData[PLUGIN_DATA_AMX_EXPORTS];
    logprintf = (logprintf_t)ppData[PLUGIN_DATA_LOGPRINTF];

	gCore = boost::shared_ptr<amxCore>(new amxCore());

	gDebug->Log("\tDebugging started\n");
	gDebug->Log("-----------------------------------------------------------------");
	gDebug->Log("Called plugin load | amx data: 0x%x | logprintf address: 0x%x", ppData[PLUGIN_DATA_AMX_EXPORTS], ppData[PLUGIN_DATA_LOGPRINTF]);
	gDebug->Log("-----------------------------------------------------------------\n");

	logprintf(" SAMP-Addon was loaded");

    return true;
}



PLUGIN_EXPORT void PLUGIN_CALL Unload()
{
	gDebug->Log("-----------------------------------------------------------------");
	gDebug->Log("Called plugin unload | amx data: 0x%x | logprintf address: 0x%x", pAMXFunctions, logprintf);
	gDebug->Log("-----------------------------------------------------------------");
	gDebug->Log("\tDebugging stopped");

    logprintf(" SAMP-Addon was unloaded");
}



PLUGIN_EXPORT int PLUGIN_CALL AmxLoad(AMX *amx) 
{
	gCore->amxList.push_back(amx);

    return amx_Register(amx, amxNatives::addonNatives, -1);
}



PLUGIN_EXPORT int PLUGIN_CALL AmxUnload(AMX *amx) 
{
	for(std::list<AMX *>::iterator i = gCore->amxList.begin(); i != gCore->amxList.end(); i++)
	{
		if(*i == amx)
		{
			gCore->amxList.erase(i);

			break;
		}
	}

    return AMX_ERR_NONE;
}



PLUGIN_EXPORT void PLUGIN_CALL ProcessTick()
{

}