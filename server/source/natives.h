#pragma once



#include "addon.h"



#define arguments(a) \
	!(params[0] != (a << 2))





AMX_NATIVE_INFO addonNatives[];



cell AMX_NATIVE_CALL n_addon_init(AMX *amx, cell *params);