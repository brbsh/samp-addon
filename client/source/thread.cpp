#pragma once

#include "thread.h"





addonThread::addonThread()
{
	addonDebug("Thread constructor called");
	addonDebug("Starting thread 'main_thread'");

	this->threadHandle = this->Start((LPTHREAD_START_ROUTINE)main_thread, (void *)GetTickCount());
}



addonThread::~addonThread()
{
	addonDebug("Thread deconstructor called");

	this->Stop(this->threadHandle);
}



HANDLE addonThread::Start(LPTHREAD_START_ROUTINE function, void *param)
{
	HANDLE threadH = CreateThread(NULL, NULL, function, param, NULL, NULL);

	addonDebug("Thread with handle %i successfuly started", threadH);

	return threadH;
}



void addonThread::Stop(HANDLE threadHandle)
{
	addonDebug("Thread with handle %i was stopped", threadHandle);

	TerminateThread(threadHandle, NULL);
	CloseHandle(threadHandle);
}