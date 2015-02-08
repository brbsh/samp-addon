#pragma once



#include "functions.hpp"





boost::shared_ptr<addonFunctions> gFunctions;


extern boost::shared_ptr<addonDebug> gDebug;





addonFunctions::addonFunctions()
{
	gDebug->traceLastFunction("addonFunctions::addonFunctions() at 0x?????");
	gDebug->Log("Called functions constructor");

	VirtualProtect((LPVOID)0x401000, 0x4A3000, PAGE_EXECUTE_READWRITE, &this->protFlags);
}



addonFunctions::~addonFunctions()
{
	gDebug->traceLastFunction("addonFunctions::~addonFunctions() at 0x?????");
	gDebug->Log("Called functions destructor");

	DWORD null = NULL;

	VirtualProtect((LPVOID)0x401000, 0x4A3000, this->protFlags, &null);
}



void addonFunctions::ToggleMotionBlur(bool status)
{
	gDebug->traceLastFunction("addonFunctions::ToggleMotionBlur(status = %s) at 0x%x", ((status) ? ("true") : ("false")),  &addonFunctions::ToggleMotionBlur);

	if(status)
	{
		// Motion blur: ON
		GTA_BLUR_ADDR_1 = 0xE8;
		GTA_BLUR_ADDR_2 = 0x11;
		GTA_BLUR_ADDR_3 = 0xE2;
		GTA_BLUR_ADDR_4 = 0xFF;
		GTA_BLUR_ADDR_5 = 0xFF;
	}
	else
	{
		// Motion blur: OFF
		GTA_BLUR_ADDR_1 = 0x90;
		GTA_BLUR_ADDR_2 = 0x90;
		GTA_BLUR_ADDR_3 = 0x90;
		GTA_BLUR_ADDR_4 = 0x90;
		GTA_BLUR_ADDR_5 = 0x90;
	}
}



void addonFunctions::ToggleGrassRendering(bool status)
{
	gDebug->traceLastFunction("addonFunctions::ToggleGrassRendering(status = %s) at 0x%x", ((status) ? ("true") : ("false")), &addonFunctions::ToggleGrassRendering);

	if(status)
	{
		// Grass rendering: ON
		GTA_GRASS_ADDR_1 = 0xE8;
		GTA_GRASS_ADDR_2 = 0x42;
		GTA_GRASS_ADDR_3 = 0x0E;
		GTA_GRASS_ADDR_4 = 0x0A;
		GTA_GRASS_ADDR_5 = 0x00;
	}
	else
	{
		// Grass rendering: OFF
		GTA_GRASS_ADDR_1 = 0x90;
		GTA_GRASS_ADDR_2 = 0x90;
		GTA_GRASS_ADDR_3 = 0x90;
		GTA_GRASS_ADDR_4 = 0x90;
		GTA_GRASS_ADDR_5 = 0x90;
	}
}