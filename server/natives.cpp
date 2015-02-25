#pragma once



#include "server.hpp"





extern logprintf_t logprintf;

extern boost::shared_ptr<amxPool> gPool;
extern boost::shared_ptr<amxSocket> gSocket;





const AMX_NATIVE_INFO amxNatives::addonNatives[] =
{
	{"Addon_StartTCPServer", amxNatives::Addon_StartTCPServer}, // native Addon_StartTCPServer(const ip[], port, maxclients);
	{"Addon_StopTCPServer", amxNatives::Addon_StopTCPServer}, // native Addon_StopTCPServer();
	{"Addon_AddClient", amxNatives::Addon_AddClient}, // native Addon_AddClient(playerid, const ip[]);
	{"Addon_RemoveClient", amxNatives::Addon_RemoveClient}, // native Addon_RemoveClient(playerid);
	{"Addon_GetActiveClients", amxNatives::Addon_GetActiveClients}, // native Addon_GetActiveClients();
	{"Addon_IsClientConnected", amxNatives::Addon_IsClientConnected}, // native Addon_IsClientConnected(clientid);
	{"Addon_GetClientIP", amxNatives::Addon_GetClientIP}, // native Addon_GetClientIP(clientid, buffer[], size = sizeof(buffer));
	{"Addon_GetClientName", amxNatives::Addon_GetClientName}, // native Addon_GetClientName(clientid, buffer[], size = sizeof(buffer));
	{"Addon_KickClient", amxNatives::Addon_KickClient}, // native Addon_KickClient(clientid);
	{"Addon_GetClientSerial", amxNatives::Addon_GetClientSerial}, // native Addon_GetClientSerial(clientid, buffer[], size = sizeof(buffer));
	{"Addon_TakeClientScreenshot", amxNatives::Addon_TakeClientScreenshot}, // native Addon_TakeClientScreenshot(clientid, const remote_filename[]);
	{"Addon_TransferLocalFile", amxNatives::Addon_TransferLocalFile}, // native Addon_TransferLocalFile(const filename[], toclient, const remote_filename[]);
	{"Addon_TransferRemoteFile", amxNatives::Addon_TransferRemoteFile}, // native Addon_TransferRemoteFile(const remote_filename[], fromclient, const filename[]);

	{NULL, NULL}
};



// native Addon_StartTCPServer(const ip[], port, maxplayers);
cell AMX_NATIVE_CALL amxNatives::Addon_StartTCPServer(AMX *amx, cell *params)
{
	if(!arguments(3))
	{
		logprintf("SAMP-Addon: Invalid argument count in native 'Addon_StartTCPServer' (%i)", (params[0] >> 4));

		return NULL;
	}

	std::string ip = amxString::Get(amx, params[1]);
	int port = params[2];
	int maxplayers = params[3];

	if(!ip.length() || (ip.length() > 15))
	{
		logprintf("SAMP-Addon: NULL ip argument passed to native 'Addon_StartTCPServer'");

		return NULL;
	}

	if((port < 1024) || (port > 65535))
	{
		logprintf("SAMP-Addon: Invalid port value passed to native 'Addon_StartTCPServer' (%i)", port);

		return NULL;
	}

	if((maxplayers < 0) || (maxplayers > 1000))
	{
		logprintf("SAMP-Addon: Invalid max players value passed to native 'Addon_StartTCPServer' (%i)", maxplayers);

		return NULL;
	}

	if(gPool->getPluginStatus())
	{
		logprintf("SAMP-Addon: Duplicate init interrupted");

		return NULL;
	}

	logprintf("\n   SAMP-Addon: Requested TCP server start on address: %s:%i (maxclients: %i)\n", ip.c_str(), ++port, maxplayers);

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



// native Addon_StopTCPServer();
cell AMX_NATIVE_CALL amxNatives::Addon_StopTCPServer(AMX *amx, cell *params)
{
	if(!arguments(0))
	{
		logprintf("SAMP-Addon: Invalid argument count in native 'Addon_StopTCPServer' (%i)", (params[0] >> 2));

		return NULL;
	}

	return 1;
}



// native Addon_AddClient(playerid, const ip[]);
cell AMX_NATIVE_CALL amxNatives::Addon_AddClient(AMX *amx, cell *params)
{
	if(!arguments(2))
	{
		logprintf("SAMP-Addon: Invalid argument count in native 'Addon_AddClient' (%i)", (params[0] >> 2));

		return NULL;
	}

	int playerid = params[1];
	std::string ip = amxString::Get(amx, params[2]);

	gSocket->getServer()->sessionAdd(playerid, boost::asio::ip::address::from_string(ip));

	return 1;
}



// native Addon_RemoveClient(playerid);
cell AMX_NATIVE_CALL amxNatives::Addon_RemoveClient(AMX *amx, cell *params)
{
	if(!arguments(1))
	{
		logprintf("SAMP-Addon: Invalid argument count in native 'Addon_RemoveClient' (%i)", (params[0] >> 2));

		return NULL;
	}

	int playerid = params[1];

	gSocket->getServer()->sessionRemove(playerid);

	return 1;
}



// native Addon_GetActiveClients();
cell AMX_NATIVE_CALL amxNatives::Addon_GetActiveClients(AMX *amx, cell *params)
{
	if(!arguments(0))
	{
		logprintf("SAMP-Addon: Invalid argument count in native 'Addon_GetActiveClients' (%i)", (params[0] >> 2));

		return NULL;
	}

	return gPool->activeSessions();
}



// native Addon_IsClientConnected(clientid);
cell AMX_NATIVE_CALL amxNatives::Addon_IsClientConnected(AMX *amx, cell *params)
{
	if(!arguments(1))
	{
		logprintf("SAMP-Addon: Invalid argument count in native 'Addon_IsClientConnected' (%i)", (params[0] >> 2));

		return NULL;
	}

	return (cell)gSocket->IsClientConnected(params[1]);
}



// native Addon_GetClientIP(clientid, buffer[], size = sizeof(buffer));
cell AMX_NATIVE_CALL amxNatives::Addon_GetClientIP(AMX *amx, cell *params)
{
	if(!arguments(3))
	{
		logprintf("SAMP-Addon: Invalid argument count in native 'Addon_GetClientIP' (%i)", (params[0] >> 2));

		return NULL;
	}

	int clientid = params[1];

	if(!gSocket->IsClientConnected(clientid))
		return NULL;

	amxString::Set(amx, params[2], gPool->getClientSession(clientid)->pool().ip.to_string(), params[3]);

	return 1;
}



// native Addon_GetClientName(clientid, buffer[], size = sizeof(buffer));
cell AMX_NATIVE_CALL amxNatives::Addon_GetClientName(AMX *amx, cell *params)
{
	if(!arguments(3))
	{
		logprintf("SAMP-Addon: Invalid argument count in native 'Addon_GetClientName' (%i)", (params[0] >> 2));

		return NULL;
	}

	int clientid = params[1];

	if(!gSocket->IsClientConnected(clientid))
		return NULL;

	amxString::Set(amx, params[2], gPool->getClientSession(clientid)->pool().name, params[3]);

	return 1;
}



// native Addon_KickClient(clientid);
cell AMX_NATIVE_CALL amxNatives::Addon_KickClient(AMX *amx, cell *params)
{
	if(!arguments(1))
	{
		logprintf("SAMP-Addon: Invalid argument count in native 'Addon_KickClient' (%i)", (params[0] >> 2));

		return NULL;
	}

	gSocket->KickClient(params[1], "amxServer::AMX: Kicked from AMX");

	return 1;
}



// native Addon_GetClientSerial(clientid, buffer[], size = sizeof(buffer));
cell AMX_NATIVE_CALL amxNatives::Addon_GetClientSerial(AMX *amx, cell *params)
{
	if(!arguments(3))
	{
		logprintf("SAMP-Addon: Invalid argument count in native 'Addon_GetClientSerial' (%i)", (params[0] >> 2));

		return NULL;
	}

	int clientid = params[1];

	if(!gSocket->IsClientConnected(clientid))
		return NULL;

	amxString::Set(amx, params[2], gPool->getClientSession(clientid)->pool().sID, params[3]);

	return 1;
}



// native Addon_TakeClientScreenshot(clientid, const remote_filename[]);
cell AMX_NATIVE_CALL amxNatives::Addon_TakeClientScreenshot(AMX *amx, cell *params)
{
	if(!arguments(2))
	{
		logprintf("SAMP-Addon: Invalid argument count in native 'Addon_TakeClientScreenshot' (%i)", (params[0] >> 2));

		return NULL;
	}

	int clientid = params[1];

	if(!gSocket->IsClientConnected(clientid))
		return NULL;

	std::string filename = amxString::Get(amx, params[2]);

	/*if((filename.find("|") != std::string::npos) || (filename.find(".png") != (filename.length() - 4)) || boost::regex_search(filename, boost::regex("[.:%]{1,2}[/\\]+")))
	{
		logprintf("SAMP-Addon: Invalid file name format");

		return NULL;
	}*/

	gPool->getClientSession(clientid)->pool().cmdresponse_state = ADDON_CMD_QUERY_SCREENSHOT;
	amxAsyncSession::writeTo(clientid, boost::str(boost::format("CMDQUERY|%1%|%2%") % ADDON_CMD_QUERY_SCREENSHOT % filename));

	return 1;
}



// native Addon_TransferLocalFile(const filename[], toclient, const remote_filename[]);
cell AMX_NATIVE_CALL amxNatives::Addon_TransferLocalFile(AMX *amx, cell *params)
{
	if(!arguments(3))
	{
		logprintf("SAMP-Addon: Invalid argument count in native 'Addon_TransferLocalFile' (%i)", (params[0] >> 2));

		return NULL;
	}

	int clientid = params[2];

	if(!gSocket->IsClientConnected(clientid))
		return NULL;

	std::string filename = amxString::Get(amx, params[1]);
	std::string remote_filename = amxString::Get(amx, params[3]);

	boost::filesystem::path fpath(filename);

	if(!boost::filesystem::exists(fpath))
	{
		return NULL;
	}

	/*if((filename.find("|") != std::string::npos) || (remote_filename.find("|") != std::string::npos) || boost::regex_search(filename, boost::regex("[/.:%]{1,2}[/\\]+")) || boost::regex_search(remote_filename, boost::regex("[.:%]{1,2}[/\\]+")))
	{
		return NULL;
	}*/

	amxAsyncSession *sess = gPool->getClientSession(clientid);
	sess->pool().cmdresponse_state = ADDON_CMD_QUERY_TLF;

	amxAsyncSession::writeTo(clientid, boost::str(boost::format("CMDQUERY|%1%|%2%|%3%|%4%") % ADDON_CMD_QUERY_TLF % remote_filename % boost::filesystem::file_size(fpath) % amxHash::crc32_file(filename)));

	sess->pool().file_t = true;
	sess->pool().fileT = new amxTransfer(true, clientid, filename, remote_filename, NULL, NULL);

	return 1;
}



// native TransferRemoteFile(const remote_filename[], fromclient, const filename[]);
cell AMX_NATIVE_CALL amxNatives::Addon_TransferRemoteFile(AMX *amx, cell *params)
{
	if(!arguments(3))
	{
		logprintf("SAMP-Addon: Invalid argument count in native 'Addon_TransferRemoteFile' (%i)", (params[0] >> 2));

		return NULL;
	}

	int clientid = params[2];

	if(!gSocket->IsClientConnected(clientid))
		return NULL;

	std::string remote_filename = amxString::Get(amx, params[1]);
	std::string filename = amxString::Get(amx, params[3]);

	boost::filesystem::path fpath(filename);

	if(boost::filesystem::exists(fpath))
	{
		return NULL;
	}

	/*if((filename.find("|") != std::string::npos) || (remote_filename.find("|") != std::string::npos) || boost::regex_search(filename, boost::regex("[/.:%]{1,2}[/\\]+")) || boost::regex_search(remote_filename, boost::regex("[.:%]{1,2}[/\\]+")))
	{
		return NULL;
	}*/

	amxAsyncSession *sess = gPool->getClientSession(clientid);
	sess->pool().cmdresponse_state = ADDON_CMD_QUERY_TRF;
	sess->pool().file_tmp_name.assign(filename);

	amxAsyncSession::writeTo(clientid, boost::str(boost::format("CMDQUERY|%1%|%2%") % ADDON_CMD_QUERY_TRF % remote_filename));

	return 1;
}