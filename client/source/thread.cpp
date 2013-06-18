#pragma once



#include "thread.h"





addonThread *gThread;





addonThread::addonThread()
{
	addonDebug("Thread constructor called");
	addonDebug("Starting thread addonThread::Thread");

	this->threadHandle = this->Start(addonThread::Thread, (void *)GetTickCount());
}



addonThread::~addonThread()
{
	addonDebug("Thread deconstructor called");

	this->Stop(this->threadHandle);
}



HANDLE addonThread::Start(threadFunc function, void *param)
{
	HANDLE threadH = CreateThread(NULL, NULL, (LPTHREAD_START_ROUTINE)function, param, NULL, NULL);

	addonDebug("Thread with handle %i successfuly started", threadH);

	return threadH;
}



void addonThread::Stop(HANDLE threadHandle)
{
	addonDebug("Thread with handle %i was stopped", threadHandle);

	TerminateThread(threadHandle, NULL);
	CloseHandle(threadHandle);
}