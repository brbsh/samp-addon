#pragma once



#define UNIQUE_HEX_MOD					std::hex << std::uppercase << std::setw(8) << std::setfill('0')
// Engine address pool
#define GTA_PED_STATUS_ADDR				(*(int *)0xB6F5F0)

// Motion blur
#define GTA_BLUR_ADDR_1					(*(BYTE *)0x704E8A)
#define GTA_BLUR_ADDR_2					(*(BYTE *)0x704E8B)
#define GTA_BLUR_ADDR_3					(*(BYTE *)0x704E8C)
#define GTA_BLUR_ADDR_4					(*(BYTE *)0x704E8D)
#define GTA_BLUR_ADDR_5					(*(BYTE *)0x704E8E)

// Grass rendering
#define GTA_GRASS_ADDR_1				(*(BYTE *)0x53C159)
#define GTA_GRASS_ADDR_2				(*(BYTE *)0x53C15A)
#define GTA_GRASS_ADDR_3				(*(BYTE *)0x53C15B)
#define GTA_GRASS_ADDR_4				(*(BYTE *)0x53C15C)
#define GTA_GRASS_ADDR_5				(*(BYTE *)0x53C15D)

// HUD toggle
#define GTA_HUD_ADDR					(*(BYTE *)0xA444A0)

// SeaWays :D (riding on water)
#define GTA_SEAWAYS_ADDR				(*(BYTE *)0x969152)

// Game speed
#define GTA_GAME_SPEED_ADDR				(*(float *)0xB7CB64)


// Queues to server
#define ADDON_CMD_QUERY_SCREENSHOT			(1000)
#define ADDON_CMD_QUERY_TLF					(1001)
#define ADDON_CMD_QUERY_TRF					(1002)



class addonCore;
class addonD3Device;
class addonDebug;
class addonFunctions;
class addonAsyncHTTP;
class addonLoader;
class addonPool;
class addonSocket;