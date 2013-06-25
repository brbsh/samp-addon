#pragma once



#include "addon.h"





class amxNatives
{

public:

	static const AMX_NATIVE_INFO addonNatives[];

	static cell AMX_NATIVE_CALL Init(AMX *amx, cell *params);
	static cell AMX_NATIVE_CALL IsClientConnected(AMX *amx, cell *params);
	static cell AMX_NATIVE_CALL KickClient(AMX *amx, cell *params);
	static cell AMX_NATIVE_CALL GetClientSerial(AMX *amx, cell *params);
	static cell AMX_NATIVE_CALL GetClientScreenshot(AMX *amx, cell *params);
	static cell AMX_NATIVE_CALL TransferLocalFile(AMX *amx, cell *params);
	static cell AMX_NATIVE_CALL TransferRemoteFile(AMX *amx, cell *params);
};