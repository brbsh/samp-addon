#pragma once



#include "addon.h"





class addonMutex
{

public:

	addonMutex();
	~addonMutex();

	void Lock();
	void unLock();

private:

	HANDLE mutexHandle;

	HANDLE Create();
	void Delete();
};