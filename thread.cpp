#pragma once

#include "thread.h"


extern addonMutex* gMutex;





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

	this->mutexHandle = this->Create(std::string("main_thread"));
}



addonMutex::~addonMutex()
{
	addonDebug("Mutex deconstructor called");

	this->Delete(this->mutexHandle);
}



HANDLE addonMutex::Create(std::string mutexcode)
{
	HANDLE mutexH = CreateMutex(NULL, FALSE, (LPCWSTR)mutexcode.c_str());

	addonDebug("Mutex with handle %i successfuly started", mutexH);

	return mutexH;
}



void addonMutex::Delete(HANDLE mutexHandle)
{
	addonDebug("Mutex with handle %i was stopped", mutexHandle);

	this->unLock(mutexHandle);
	CloseHandle(mutexHandle);
}



void addonMutex::Lock(HANDLE mutexHandle)
{
	WaitForSingleObject(mutexHandle, INFINITE);
}



void addonMutex::unLock(HANDLE mutexHandle)
{
	ReleaseMutex(mutexHandle);
}