#pragma once



#define ADDON_CALLBACK_OTWS					(0) // Addon_OnTCPWorkerStarted(workerid);
#define ADDON_CALLBACK_OTWE					(1) // Addon_OnTCPWorkerError(workerid, error[]);
#define ADDON_CALLBACK_OCC					(2) // Addon_OnClientConnect(clientid, client_ip[]);
#define ADDON_CALLBACK_OCCE					(3) // Addon_OnClientConnectError(client_ip[], error[]);
#define ADDON_CALLBACK_OCD					(4) // Addon_OnClientDisconnect(clientid, client_ip[], reason_code, reason[]);
#define ADDON_CALLBACK_OCST					(5) // Addon_OnClientScreenshotTaken(clientid, remote_filename[]);
#define ADDON_CALLBACK_OCSE					(6) // Addon_OnClientScreenshotError(clientid, remote_filename[], error[]);
#define ADDON_CALLBACK_OCAR					(7) // Addon_OnClientAddressRequest(clientid, address, type, value[]);

#define ADDON_CLIENT_TIMEOUT_SECONDS		(30)

#define ADDON_CMD_QUERY_SCREENSHOT			(1000)
#define ADDON_CMD_QUERY_SETADDR				(1001)
#define ADDON_CMD_QUERY_GETADDR				(1002)





class amxCore;
class amxDebug;
class amxNatives;
class amxPool;
class amxSocket;
class amxAsyncServer;
class amxAsyncSession;