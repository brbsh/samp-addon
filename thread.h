#pragma once

#include "addon.h"



class addonThread
{
public:
	HANDLE threadHandle;
	
	addonThread();
	~addonThread();

	HANDLE Start(LPTHREAD_START_ROUTINE function);
	void Stop(HANDLE threadHandle);
};



class addonMutex
{
public:
	HANDLE mutexHandle;

	addonMutex();
	~addonMutex();

	HANDLE Create(std::string mutexcode);
	void Delete(HANDLE mutexHandle);
	void Lock(HANDLE mutexHandle);
	void unLock(HANDLE mutexHandle);
};