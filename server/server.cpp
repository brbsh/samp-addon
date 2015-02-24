#pragma once



#include "server.hpp"



extern void *pAMXFunctions;

extern boost::shared_ptr<amxCore> gCore;
extern boost::shared_ptr<amxDebug> gDebug;
extern boost::shared_ptr<amxPool> gPool;
extern boost::shared_ptr<amxSocket> gSocket;
extern boost::shared_ptr<amxTransfer> gTransfer;


logprintf_t logprintf;





PLUGIN_EXPORT unsigned int PLUGIN_CALL Supports() 
{
    return (SUPPORTS_VERSION | SUPPORTS_AMX_NATIVES | SUPPORTS_PROCESS_TICK);
}



PLUGIN_EXPORT bool PLUGIN_CALL Load(void **ppData) 
{
    pAMXFunctions = ppData[PLUGIN_DATA_AMX_EXPORTS];
    logprintf = (logprintf_t)ppData[PLUGIN_DATA_LOGPRINTF];

	gDebug = boost::shared_ptr<amxDebug>(new amxDebug());

	gDebug->Log("\tDebugging started\n");
	gDebug->Log("-----------------------------------------------------------------");
	gDebug->Log("Called plugin load | amx data: 0x%x | logprintf address: 0x%x", ppData[PLUGIN_DATA_AMX_EXPORTS], logprintf);
	gDebug->Log("-----------------------------------------------------------------\n");

	gCore = boost::shared_ptr<amxCore>(new amxCore());

	#if defined LINUX
	boost::filesystem::path tmppath("plugins/_addon_tmp.so");
	boost::filesystem::path libpath("plugins/addon.so");
	boost::filesystem::path curcfg("server.cfg");
	boost::filesystem::path tmpcfg("server.cfg.bak");
	#else
	boost::filesystem::path tmppath(".\\plugins\\_addon_tmp.dll");
	boost::filesystem::path libpath(".\\plugins\\addon.dll");
	boost::filesystem::path curcfg(".\\server.cfg");
	boost::filesystem::path tmpcfg(".\\server.cfg.bak");
	#endif

	if(boost::filesystem::exists(tmppath) && boost::filesystem::exists(tmpcfg))
	{
		try
		{
			boost::filesystem::remove(libpath);
			boost::filesystem::copy_file(tmppath, libpath);

			boost::filesystem::remove(curcfg);
			boost::filesystem::rename(tmpcfg, curcfg);
		}
		catch(boost::filesystem::filesystem_error& error)
		{
			logprintf("SAMP-Addon warning: When updating addon binary: %s", error.what());
		}

		logprintf("SAMP-Addon warning: Addon binary was updated. Start server again.");
		exit(EXIT_SUCCESS);
	}
	else if(boost::filesystem::exists(tmppath) && !boost::filesystem::exists(tmpcfg))
	{
		try
		{
			boost::filesystem::remove(tmppath);
		}
		catch(boost::filesystem::filesystem_error& error)
		{
			logprintf("SAMP-Addon warning: Cannot remove old addon binary, remove it manually (What: %s)", error.what());
		}

		logprintf("SAMP-Addon warning: old addon binary was removed");
	}

	logprintf(" SAMP-Addon was loaded");

    return true;
}



PLUGIN_EXPORT void PLUGIN_CALL Unload()
{
	gDebug->Log("-----------------------------------------------------------------");
	gDebug->Log("Called plugin unload | amx data: 0x%x | logprintf address: 0x%x", pAMXFunctions, logprintf);
	gDebug->Log("-----------------------------------------------------------------");
	gDebug->Log("\tDebugging stopped");

	gPool->setPluginStatus(false);

	gCore.reset();
	gDebug.reset();

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
	if(gCore->isPTEmpty())
		return;

	int idx = NULL;
	cell amxAddr[3];
	std::pair<unsigned int, amxCore::amxPush> data = gCore->getFromPT();
	amxPool::svrData fData;

	for(std::list<AMX *>::iterator i = gCore->amxList.begin(); i != gCore->amxList.end(); i++)
	{
		switch(data.first)
		{
			case ADDON_CALLBACK_OTWS: // Addon_OnTCPWorkerStarted(workerid);
			{
				if(!amx_FindPublic(*i, "Addon_OnTCPWorkerStarted", &idx))
				{
					amx_Push(*i, data.second.clientid); // workerid

					amx_Exec(*i, NULL, idx);
				}
			}
			break;

			case ADDON_CALLBACK_OTWE: // Addon_OnTCPWorkerError(workerid, error_code, error[]);
			{
				if(!amx_FindPublic(*i, "Addon_OnTCPWorkerError", &idx))
				{
					fData.reset();
					fData = data.second.args.at(1);
					amx_PushString(*i, &amxAddr[0], NULL, fData.string.c_str(), NULL, NULL); // error[]

					fData.reset();
					fData = data.second.args.at(0);
					amx_Push(*i, fData.integer); // error_code

					amx_Push(*i, data.second.clientid); // workerid

					amx_Exec(*i, NULL, idx);
					amx_Release(*i, amxAddr[0]);
				}
			}
			break;

			case ADDON_CALLBACK_OCC: // Addon_OnClientConnect(clientid, client_ip[]);
			{
				if(!amx_FindPublic(*i, "Addon_OnClientConnect", &idx))
				{
					fData.reset();
					fData = data.second.args.at(0);
					amx_PushString(*i, &amxAddr[0], NULL, fData.string.c_str(), NULL, NULL); // client_ip[]

					amx_Push(*i, data.second.clientid); // clientid

					amx_Exec(*i, NULL, idx);
					amx_Release(*i, amxAddr[0]);
				}
			}
			break;

			case ADDON_CALLBACK_OCCE: // Addon_OnClientConnectError(client_ip[], error_code, error[]);
			{
				if(!amx_FindPublic(*i, "Addon_OnClientConnectError", &idx))
				{
					fData.reset();
					fData = data.second.args.at(2);
					amx_PushString(*i, &amxAddr[0], NULL, fData.string.c_str(), NULL, NULL); // error[]
					
					fData.reset();
					fData = data.second.args.at(1);
					amx_Push(*i, fData.integer); // error_code

					fData.reset();
					fData = data.second.args.at(0);
					amx_PushString(*i, &amxAddr[1], NULL, fData.string.c_str(), NULL, NULL); // client_ip[]

					amx_Exec(*i, NULL, idx);
					amx_Release(*i, amxAddr[1]);
					amx_Release(*i, amxAddr[0]);
				}
			}
			break;

			case ADDON_CALLBACK_OCD: // Addon_OnClientDisconnect(clientid, client_ip[], reason_code, reason[]);
			{
				if(!amx_FindPublic(*i, "Addon_OnClientDisconnect", &idx))
				{
					fData.reset();
					fData = data.second.args.at(2);
					amx_PushString(*i, &amxAddr[0], NULL, fData.string.c_str(), NULL, NULL); // reason[]

					fData.reset();
					fData = data.second.args.at(1);
					amx_Push(*i, fData.integer); // reason_code

					fData.reset();
					fData = data.second.args.at(0);
					amx_PushString(*i, &amxAddr[1], NULL, fData.string.c_str(), NULL, NULL); // client_ip[]

					amx_Push(*i, data.second.clientid); // clientid

					amx_Exec(*i, NULL, idx);
					amx_Release(*i, amxAddr[1]);
					amx_Release(*i, amxAddr[0]);
				}
			}
			break;

			case ADDON_CALLBACK_OCST: // Addon_OnClientScreenshotTaken(clientid, remote_filename[]);
			{
				if(!amx_FindPublic(*i, "Addon_OnClientScreenshotTaken", &idx))
				{
					fData.reset();
					fData = data.second.args.at(0);
					amx_PushString(*i, &amxAddr[0], NULL, fData.string.c_str(), NULL, NULL); // remote_filename[]

					amx_Push(*i, data.second.clientid); // clientid

					amx_Exec(*i, NULL, idx);
					amx_Release(*i, amxAddr[0]);
				}
			}
			break;

			case ADDON_CALLBACK_OCSE: // Addon_OnClientScreenshotError(clientid, remote_filename[], error[]);
			{
				if(!amx_FindPublic(*i, "Addon_OnClientScreenshotError", &idx))
				{
					fData.reset();
					fData = data.second.args.at(1);
					amx_PushString(*i, &amxAddr[0], NULL, fData.string.c_str(), NULL, NULL); // error[]

					fData.reset();
					fData = data.second.args.at(0);
					amx_PushString(*i, &amxAddr[1], NULL, fData.string.c_str(), NULL, NULL); // remote_filename[]

					amx_Push(*i, data.second.clientid); // clientid

					amx_Exec(*i, NULL, idx);
					amx_Release(*i, amxAddr[1]);
					amx_Release(*i, amxAddr[0]);
				}
			}
			break;

			case ADDON_CALLBACK_OLFT: // Addon_OnLocalFileTransfered(filename[], clientid, remote_filename[], bytes_transfered);
			{
				// some dirty cleanup
				amxTransfer *pointer = gPool->getClientSession(data.second.clientid)->pool().fileT;
				delete pointer;

				if(!amx_FindPublic(*i, "Addon_OnLocalFileTransfered", &idx))
				{
					fData.reset();
					fData = data.second.args.at(2);
					amx_Push(*i, fData.integer); // bytes_transfered

					fData.reset();
					fData = data.second.args.at(1);
					amx_PushString(*i, &amxAddr[0], NULL, fData.string.c_str(), NULL, NULL); // remote_filename[]

					amx_Push(*i, data.second.clientid); // clientid

					fData.reset();
					fData = data.second.args.at(0);
					amx_PushString(*i, &amxAddr[1], NULL, fData.string.c_str(), NULL, NULL); // filename[]

					amx_Exec(*i, NULL, idx);
					amx_Release(*i, amxAddr[1]);
					amx_Release(*i, amxAddr[0]);
				}
			}
			break;

			case ADDON_CALLBACK_OLFTE: // Addon_OnLocalFileTransferError(filename[], clientid, remote_filename[], error_code, error[]);
			{
				// some dirty cleanup
				amxTransfer *pointer = gPool->getClientSession(data.second.clientid)->pool().fileT;
				delete pointer;

				if(!amx_FindPublic(*i, "Addon_OnLocalFileTransferError", &idx))
				{
					fData.reset();
					fData = data.second.args.at(3);
					amx_PushString(*i, &amxAddr[0], NULL, fData.string.c_str(), NULL, NULL); // error[]

					fData.reset();
					fData = data.second.args.at(2);
					amx_Push(*i, fData.integer); // error_code

					fData.reset();
					fData = data.second.args.at(1);
					amx_PushString(*i, &amxAddr[1], NULL, fData.string.c_str(), NULL, NULL); // remote_filename[]

					amx_Push(*i, data.second.clientid); // clientid

					fData.reset();
					fData = data.second.args.at(0);
					amx_PushString(*i, &amxAddr[2], NULL, fData.string.c_str(), NULL, NULL); // filename[]

					amx_Exec(*i, NULL, idx);
					amx_Release(*i, amxAddr[2]);
					amx_Release(*i, amxAddr[1]);
					amx_Release(*i, amxAddr[0]);
				}
			}
			break;
		}
	}
}