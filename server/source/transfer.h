#pragma once



#include "addon.h"





class amxTransfer
{

public:

	#ifdef WIN32
		static DWORD SendThread(void *lpParam);
		static DWORD ReceiveThread(void *lpParam);
	#else
		static void *SendThread(void *lpParam);
		static void *ReceiveThread(void *lpParam);
	#endif
};