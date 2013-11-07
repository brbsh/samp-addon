#pragma once



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


// Queues to server
#define ADDON_PENDING_CONNECT_QUEUE		(0x2374BA1)
#define ADDON_PENDING_DISCONNECT_QUEUE	(0x34F32A3)
#define ADDON_PENDING_SETVAR_RESPONSE	(0x8041317)

// Responses from server
#define ADDON_PENDING_CONNECT_RESPONSE	(0xD911B15)
#define ADDON_PENDING_KICK_NOTIFICATION	(0x472CD78) // DISCONNECT_RESPONSE, lolwhut?
#define ADDON_PENDING_SETVAR_QUEUE		(0x573E3FA)