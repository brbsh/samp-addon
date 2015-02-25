#pragma once



#include "server.hpp"





#define arguments(a) \
	!(params[0] != (a << 2))





class amxNatives
{

public:

	static const AMX_NATIVE_INFO addonNatives[];

	static cell AMX_NATIVE_CALL Addon_StartTCPServer(AMX *amx, cell *params);
	static cell AMX_NATIVE_CALL Addon_StopTCPServer(AMX *amx, cell *params);
	static cell AMX_NATIVE_CALL Addon_AddClient(AMX *amx, cell *params);
	static cell AMX_NATIVE_CALL Addon_RemoveClient(AMX *amx, cell *params);
	static cell AMX_NATIVE_CALL Addon_GetActiveClients(AMX *amx, cell *params);
	static cell AMX_NATIVE_CALL Addon_IsClientConnected(AMX *amx, cell *params);
	static cell AMX_NATIVE_CALL Addon_GetClientIP(AMX *amx, cell *params);
	static cell AMX_NATIVE_CALL Addon_GetClientName(AMX *amx, cell *params);
	static cell AMX_NATIVE_CALL Addon_KickClient(AMX *amx, cell *params);
	static cell AMX_NATIVE_CALL Addon_GetClientSerial(AMX *amx, cell *params);
	static cell AMX_NATIVE_CALL Addon_TakeClientScreenshot(AMX *amx, cell *params);
	static cell AMX_NATIVE_CALL Addon_TransferLocalFile(AMX *amx, cell *params);
	static cell AMX_NATIVE_CALL Addon_TransferRemoteFile(AMX *amx, cell *params);
};