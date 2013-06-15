#pragma once



#include "thread.h"



amxThread *gThread;





#ifdef WIN32
	HANDLE amxThread::Start(threadFunc function, void *param)
#else
	pthread_t amxThread::Start(threadFunc function, void *param)
#endif
{
	#ifdef WIN32
		HANDLE threadHandle = CreateThread(NULL, NULL, (LPTHREAD_START_ROUTINE)function, param, NULL, NULL);
	#else
		pthread_t threadHandle;
		pthread_attr_t attr;

		pthread_attr_init(&attr);
		pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_DETACHED);
		pthread_create(&threadHandle, &attr, function, param);
	#endif

	return threadHandle;
}



#ifdef WIN32
	void amxThread::Stop(HANDLE thread)
#else
	void amxThread::Stop(pthread_t thread)
#endif
{
	#ifdef WIN32
		TerminateThread(thread, NULL);
	#else
		pthread_join(thread, NULL);
	#endif
}