#pragma once

#include "thread.h"




addonThread::addonThread()
{
	addonDebug("Thread constructor called");
	addonDebug("Starting thread 'main_thread'");

	this->threadHandle = this->Start((LPTHREAD_START_ROUTINE)main_thread);
}



addonThread::~addonThread()
{
	addonDebug("Thread deconstructor called");

	TerminateThread(this->threadHandle, NULL);
	CloseHandle(this->threadHandle);
}



HANDLE addonThread::Start(LPTHREAD_START_ROUTINE function)
{
	HANDLE threadH = CreateThread(NULL, 0, function, NULL, 0, NULL);

	addonDebug("Thread with handle %i successfuly started", threadH);

	return threadH;
}



void addonThread::Stop(HANDLE threadHandle)
{
	addonDebug("Thread with handle %i was stopped", threadHandle);

	TerminateThread(threadHandle, NULL);
	CloseHandle(threadHandle);
}



addonMutex::addonMutex()
{
	addonDebug("Mutex constructor called");

	this->mutexHandle = CreateMutex(NULL, FALSE, L"addon");
}



addonMutex::~addonMutex()
{
	addonDebug("Mutex deconstructor called");

	CloseHandle(this->mutexHandle);
}



void addonMutex::Lock()
{
	WaitForSingleObject(this->mutexHandle, INFINITE);
}



void addonMutex::unLock()
{
	ReleaseMutex(this->mutexHandle);
}