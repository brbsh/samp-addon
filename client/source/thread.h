#pragma once

#include "addon.h"



class addonThread
{

public:

	HANDLE threadHandle;
	
	addonThread();
	~addonThread();

	HANDLE Start(LPTHREAD_START_ROUTINE function, LPVOID param);
	void Stop(HANDLE threadHandle);
};