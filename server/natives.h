#pragma once



#include "server.h"


#define arguments(a) \
	!(params[0] != (a << 2))





class amxNatives
{

public:

	static const AMX_NATIVE_INFO addonNatives[];

	static cell AMX_NATIVE_CALL InitAddon(AMX *amx, cell *params);
	static cell AMX_NATIVE_CALL IsClientConnected(AMX *amx, cell *params);
	static cell AMX_NATIVE_CALL KickClient(AMX *amx, cell *params);
	static cell AMX_NATIVE_CALL GetClientSerial(AMX *amx, cell *params);
	static cell AMX_NATIVE_CALL GetClientScreenshot(AMX *amx, cell *params);
	static cell AMX_NATIVE_CALL TransferLocalFile(AMX *amx, cell *params);
	static cell AMX_NATIVE_CALL TransferRemoteFile(AMX *amx, cell *params);
};