#pragma once



#include "addon.h"



extern void *pAMXFunctions;

//extern amxMutex *gMutex;
extern amxPool *gPool;
extern amxSocket *gSocket;
//extern amxThread *gThread;

logprintf_t logprintf;


boost::mutex gMutex;
std::list<AMX *> amxList;

std::queue<amxConnect> amxConnectQueue;
std::queue<amxConnectError> amxConnectErrorQueue;
std::queue<amxDisconnect> amxDisconnectQueue;
std::queue<amxKey> amxKeyQueue;
std::queue<amxScreenshot> amxScreenshotQueue;
std::queue<amxFileReceive> amxFileReceiveQueue;
std::queue<amxFileSend> amxFileSendQueue;
std::queue<amxFileError> amxFileErrorQueue;





PLUGIN_EXPORT unsigned int PLUGIN_CALL Supports() 
{
    return (SUPPORTS_VERSION | SUPPORTS_AMX_NATIVES | SUPPORTS_PROCESS_TICK);
}



PLUGIN_EXPORT bool PLUGIN_CALL Load(void **ppData) 
{
    pAMXFunctions = ppData[PLUGIN_DATA_AMX_EXPORTS];
    logprintf = (logprintf_t)ppData[PLUGIN_DATA_LOGPRINTF];

	//gMutex = new amxMutex();
	gPool = new amxPool();
	gSocket = new amxSocket();

    logprintf(" samp-addon was loaded");

    return true;
}



PLUGIN_EXPORT void PLUGIN_CALL Unload()
{
	//delete gMutex;
	gSocket->Close();

	boost::mutex::scoped_lock lock(gPool->Mutex);
	gPool->pluginInit = false;
	lock.unlock();

    logprintf(" samp-addon was unloaded");
}



PLUGIN_EXPORT int PLUGIN_CALL AmxLoad(AMX *amx) 
{
	amxList.push_back(amx);

    return amx_Register(amx, amxNatives::addonNatives, -1);
}



PLUGIN_EXPORT int PLUGIN_CALL AmxUnload(AMX *amx) 
{
	for(std::list<AMX *>::iterator i = amxList.begin(); i != amxList.end(); i++)
	{
		if(*i == amx)
		{
			amxList.erase(i);

			break;
		}
	}

    return AMX_ERR_NONE;
}



PLUGIN_EXPORT void PLUGIN_CALL ProcessTick()
{
	if(!amxConnectQueue.empty())
	{
		int amx_idx;
		cell amxAddress;

		amxConnect connectData;

		for(unsigned int i = 0; i < amxConnectQueue.size(); i++)
		{
			boost::mutex::scoped_lock lock(gMutex);
			connectData = amxConnectQueue.front();
			amxConnectQueue.pop();
			lock.unlock();

			for(std::list<AMX *>::iterator amx = amxList.begin(); amx != amxList.end(); amx++)
			{
				// public Addon_OnClientConnect(clientid, client_ip[])
				if(!amx_FindPublic(*amx, "Addon_OnClientConnect", &amx_idx))
				{
					amx_PushString(*amx, &amxAddress, NULL, connectData.ip.c_str(), NULL, NULL);
					amx_Push(*amx, connectData.clientID);

					amx_Exec(*amx, NULL, amx_idx);

					amx_Release(*amx, amxAddress);
				}
			}
		}
	}

	if(!amxConnectErrorQueue.empty())
	{
		int amx_idx;
		cell amxAddress[2];

		amxConnectError connectErrorData;

		for(unsigned int i = 0; i < amxConnectErrorQueue.size(); i++)
		{
			boost::mutex::scoped_lock lock(gMutex);
			connectErrorData = amxConnectErrorQueue.front();
			amxConnectErrorQueue.pop();
			lock.unlock();

			for(std::list<AMX *>::iterator amx = amxList.begin(); amx != amxList.end(); amx++)
			{
				// public Addon_OnClientConnectError(client_ip[], error[], error_id)
				if(!amx_FindPublic(*amx, "Addon_OnClientConnectError", &amx_idx))
				{
					amx_Push(*amx, connectErrorData.errorCode);
					amx_PushString(*amx, &amxAddress[0], NULL, connectErrorData.error.c_str(), NULL, NULL);
					amx_PushString(*amx, &amxAddress[1], NULL, connectErrorData.ip.c_str(), NULL, NULL);

					amx_Exec(*amx, NULL, amx_idx);

					amx_Release(*amx, amxAddress[1]);
					amx_Release(*amx, amxAddress[0]);
				}
			}
		}
	}

	if(!amxDisconnectQueue.empty())
	{
		int amx_idx;

		amxDisconnect disconnectData;

		for(unsigned int i = 0; i < amxDisconnectQueue.size(); i++)
		{
			boost::mutex::scoped_lock lock(gMutex);
			disconnectData = amxDisconnectQueue.front();
			amxDisconnectQueue.pop();
			lock.unlock();

			for(std::list<AMX *>::iterator amx = amxList.begin(); amx != amxList.end(); amx++)
			{
				// public Addon_OnClientDisconnect(clientid)
				if(!amx_FindPublic(*amx, "Addon_OnClientDisconnect", &amx_idx))
				{
					amx_Push(*amx, disconnectData.clientID);

					amx_Exec(*amx, NULL, amx_idx);
				}
			}
		}
	}

	if(!amxKeyQueue.empty())
	{
		int amx_idx;
		cell amxAddress;

		amxKey keyData;

		for(unsigned int i = 0; i < amxKeyQueue.size(); i++)
		{
			boost::mutex::scoped_lock lock(gMutex);
			keyData = amxKeyQueue.front();
			amxKeyQueue.pop();
			lock.unlock();

			for(std::list<AMX *>::iterator amx = amxList.begin(); amx != amxList.end(); amx++)
			{
				// public Addon_OnClientKeyDataUpdate(clientid, keys[])
				if(!amx_FindPublic(*amx, "Addon_OnClientKeyDataUpdate", &amx_idx))
				{
					amx_PushString(*amx, &amxAddress, NULL, keyData.keys, NULL, NULL);
					amx_Push(*amx, keyData.clientID);

					amx_Exec(*amx, NULL, amx_idx);

					amx_Release(*amx, amxAddress);
				}
			}
		}
	}

	if(!amxScreenshotQueue.empty())
	{
		int amx_idx;
		cell amxAddress;

		amxScreenshot screenshotData;

		for(unsigned int i = 0; i < amxScreenshotQueue.size(); i++)
		{
			boost::mutex::scoped_lock lock(gMutex);
			screenshotData = amxScreenshotQueue.front();
			amxScreenshotQueue.pop();
			lock.unlock();

			for(std::list<AMX *>::iterator amx = amxList.begin(); amx != amxList.end(); amx++)
			{
				// public Addon_OnScreenshotTaken(clientid, remote_file[])
				if(!amx_FindPublic(*amx, "Addon_OnScreenshotTaken", &amx_idx))
				{
					amx_PushString(*amx, &amxAddress, NULL, screenshotData.name, NULL, NULL);
					amx_Push(*amx, screenshotData.clientID);

					amx_Exec(*amx, NULL, amx_idx);

					amx_Release(*amx, amxAddress);
				}
			}
		}
	}

	if(!amxFileReceiveQueue.empty())
	{
		int amx_idx;
		cell amxAddress;

		amxFileReceive fileData;

		for(unsigned int i = 0; i < amxFileReceiveQueue.size(); i++)
		{
			boost::mutex::scoped_lock lock(gMutex);
			fileData = amxFileReceiveQueue.front();
			amxFileReceiveQueue.pop();
			lock.unlock();

			for(std::list<AMX *>::iterator amx = amxList.begin(); amx != amxList.end(); amx++)
			{
				// public Addon_OnClientFileReceived(clientid, file[])
				if(!amx_FindPublic(*amx, "Addon_OnClientFileReceived", &amx_idx))
				{
					amx_PushString(*amx, &amxAddress, NULL, fileData.file.c_str(), NULL, NULL);
					amx_Push(*amx, fileData.clientid);

					amx_Exec(*amx, NULL, amx_idx);

					amx_Release(*amx, amxAddress);
				}
			}
		}
	}

	if(!amxFileSendQueue.empty())
	{
		int amx_idx;
		cell amxAddress;

		amxFileSend fileData;

		for(unsigned int i = 0; i < amxFileSendQueue.size(); i++)
		{
			boost::mutex::scoped_lock lock(gMutex);
			fileData = amxFileSendQueue.front();
			amxFileSendQueue.pop();
			lock.unlock();

			for(std::list<AMX *>::iterator amx = amxList.begin(); amx != amxList.end(); amx++)
			{
				// public Addon_OnClientFileSended(clientid, file[])
				if(!amx_FindPublic(*amx, "Addon_OnClientFileSended", &amx_idx))
				{
					amx_PushString(*amx, &amxAddress, NULL, fileData.file.c_str(), NULL, NULL);
					amx_Push(*amx, fileData.clientid);

					amx_Exec(*amx, NULL, amx_idx);

					amx_Release(*amx, amxAddress);
				}
			}
		}
	}

	if(!amxFileErrorQueue.empty())
	{
		int amx_idx;
		cell amxAddress[2];

		amxFileError fileError;

		for(unsigned int i = 0; i < amxFileErrorQueue.size(); i++)
		{
			boost::mutex::scoped_lock lock(gMutex);
			fileError = amxFileErrorQueue.front();
			amxFileErrorQueue.pop();
			lock.unlock();

			for(std::list<AMX *>::iterator amx = amxList.begin(); amx != amxList.end(); amx++)
			{
				// public Addon_OnFileTransferError(io_mode, clientid, file[], error[], error_code)
				if(!amx_FindPublic(*amx, "Addon_OnFileTransferError", &amx_idx))
				{
					amx_Push(*amx, fileError.errorCode);
					amx_PushString(*amx, &amxAddress[0], NULL, fileError.error.c_str(), NULL, NULL);
					amx_PushString(*amx, &amxAddress[1], NULL, fileError.file.c_str(), NULL, NULL);
					amx_Push(*amx, fileError.clientid);
					amx_Push(*amx, fileError.io);

					amx_Exec(*amx, NULL, amx_idx);

					amx_Release(*amx, amxAddress[1]);
					amx_Release(*amx, amxAddress[0]);
				}
			}
		}
	}
}