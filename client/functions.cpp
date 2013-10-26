#pragma once



#include "functions.h"





boost::shared_ptr<addonFunctions> gFunctions;


extern boost::shared_ptr<addonDebug> gDebug;





addonFunctions::addonFunctions()
{
	gDebug->TraceLastFunction("addonFunctions::addonFunctions() at 0x?????");

	VirtualProtect((LPVOID)0x401000, 0x4A3000, PAGE_EXECUTE_READWRITE, &this->protFlags);
}



addonFunctions::~addonFunctions()
{
	gDebug->TraceLastFunction("addonFunctions::~addonFunctions() at 0x?????");

	DWORD null = NULL;

	VirtualProtect((LPVOID)0x401000, 0x4A3000, this->protFlags, &null);
}



void addonFunctions::ToggleMotionBlur(bool status)
{
	gDebug->TraceLastFunction(strFormat() << "addonFunctions::ToggleMotionBlur(...) at 0x" << std::hex << &addonFunctions::ToggleMotionBlur);

	if(status)
	{
		// Motion blur
		*(BYTE *)0x704E8A = 0xE8;
		*(BYTE *)0x704E8B = 0x11;
		*(BYTE *)0x704E8C = 0xE2;
		*(BYTE *)0x704E8D = 0xFF;
		*(BYTE *)0x704E8E = 0xFF;
	}
	else
	{

	}
}



void addonFunctions::ToggleGrassRendering(bool status)
{
	gDebug->TraceLastFunction(strFormat() << "addonFunctions::ToggleGrassRendering(...) at 0x" << std::hex << &addonFunctions::ToggleGrassRendering);

	if(status)
	{
		// Grass
		*(BYTE *)0x53C159 = 0xE8;
		*(BYTE *)0x53C15A = 0x42;
		*(BYTE *)0x53C15B = 0x0E;
		*(BYTE *)0x53C15C = 0x0A;
		*(BYTE *)0x53C15D = 0x00;
	}
	else
	{

	}
}