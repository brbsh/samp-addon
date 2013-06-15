#pragma once



#include "addon.h"



#ifdef WIN32
	typedef DWORD (*threadFunc)(void *lpParam);
#else
	typedef void *(*threadFunc)(void *lpParam);
#endif





class amxThread
{

public:

	#ifdef WIN32
		HANDLE Start(threadFunc function, void *param);
		void Stop(HANDLE thread);
	#else
		pthread_t Start(threadFunc function, void *param);
		void Stop(pthread_t thread);
	#endif
};


