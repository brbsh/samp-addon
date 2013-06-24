#pragma once



#include "mutex.h"



amxMutex *gMutex;





amxMutex::amxMutex()
{
	#ifdef WIN32
		this->mutexHandle = CreateMutex(NULL, FALSE, L"samp-addon");
	#else
		this->mutexHandle = PTHREAD_MUTEX_INITIALIZER;
	#endif
}



amxMutex::~amxMutex()
{
	#ifdef WIN32
		CloseHandle(this->mutexHandle);
	#else
		pthread_mutex_destroy(&this->mutexHandle);
	#endif
}



void amxMutex::Lock()
{
	#ifdef WIN32
		WaitForSingleObject(this->mutexHandle, INFINITE);
	#else
		pthread_mutex_lock(&this->mutexHandle);
	#endif
}



void amxMutex::unLock()
{
	#ifdef WIN32
		ReleaseMutex(this->mutexHandle);
	#else
		pthread_mutex_unlock(&this->mutexHandle);
	#endif
}