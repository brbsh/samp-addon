#pragma once



#include "addon.h"





class amxMutex
{

public:

	amxMutex();
	~amxMutex();

	void Lock();
	void unLock();

private:

	#ifdef WIN32
		HANDLE mutexHandle;
	#else
		pthread_mutex_t mutexHandle;
	#endif
};