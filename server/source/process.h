#pragma once



#include "addon.h"





class amxProcess
{

public:

	#ifdef WIN32
		static DWORD Thread(void *lpParam);
	#else
		static void *Thread(void *lpParam);
	#endif
};