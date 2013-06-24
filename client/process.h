#pragma once



#include "addon.h"





class addonProcess
{

public:

	static DWORD Thread(void *lpParam);

	addonProcess();
	~addonProcess();

private:

	HANDLE processHandle;
};