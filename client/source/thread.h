#pragma once



#include "addon.h"



typedef DWORD (*threadFunc)(void *lpParam);





class addonThread
{

public:
	
	static DWORD Thread(void *lpParam);

	addonThread();
	~addonThread();

	HANDLE Start(threadFunc function, void *param);
	void Stop(HANDLE threadHandle);

private:

	HANDLE threadHandle;
};