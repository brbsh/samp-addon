#pragma once



#include "addon.h"





class addonTransfer
{

public:

	static DWORD SendThread(void *lpParam);
	static DWORD ReceiveThread(void *lpParam);
};