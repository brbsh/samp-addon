#pragma once

#include "mouselog.h"


extern addonThread* gThread;
extern addonMutex* gMutex;
extern addonMouselog* gMouselog;

std::queue<POINT> mouseQueue;





addonMouselog::addonMouselog()
{
	addonDebug("Mouselog constructor called");

	this->active = true;

	this->mouselogHandle = gThread->Start((LPTHREAD_START_ROUTINE)mouselog_thread);
}



addonMouselog::~addonMouselog()
{
	addonDebug("Mouselog deconstructor called");

	this->active = false;

	gThread->Stop(this->mouselogHandle);
}



DWORD _stdcall mouselog_thread(LPVOID lpParam)
{
	addonDebug("Thread 'mouselog_thread' successfuly started");

	POINT currentPoint, prevPoint;

	while(gMouselog->active)
	{
		GetCursorPos(&currentPoint);

		if(prevPoint.x != currentPoint.x || prevPoint.y != currentPoint.y)
		{
			gMutex->Lock(gMutex->mutexHandle);
			mouseQueue.push(currentPoint);
			gMutex->unLock(gMutex->mutexHandle);
			
			prevPoint = currentPoint;
		}

		Sleep(500);
	}

	return true;
}
