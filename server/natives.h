#pragma once



#include "addon.h"



#define arguments(a) \
	!(params[0] != (a << 2))





AMX_NATIVE_INFO addonNatives[];



cell AMX_NATIVE_CALL n_addon_init(AMX *amx, cell *params);
cell AMX_NATIVE_CALL n_addon_is_client_connected(AMX *amx, cell *params);
cell AMX_NATIVE_CALL n_addon_kick_client(AMX *amx, cell *params);

cell AMX_NATIVE_CALL n_addon_get_serial(AMX *amx, cell *params);
cell AMX_NATIVE_CALL n_addon_get_screenshot(AMX *amx, cell *params);

cell AMX_NATIVE_CALL n_addon_send_file(AMX *amx, cell *params);
cell AMX_NATIVE_CALL n_addon_get_file(AMX *amx, cell *params);