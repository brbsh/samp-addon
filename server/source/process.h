#pragma once



#include "addon.h"





#ifdef WIN32
	DWORD process_thread(void *lpParam);
#else
	void *process_thread(void *lpParam);
#endif