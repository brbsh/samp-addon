#pragma once

#include "thread.h"



extern addonMutex *gMutex;





addonThread::addonThread()
{
	addonDebug("Thread constructor called");
	addonDebug("Starting thread 'main_thread'");

	this->threadHandle = this->Start((LPTHREAD_START_ROUTINE)main_thread, (LPVOID)GetTickCount());
}



addonThread::~addonThread()
{
	addonDebug("Thread deconstructor called");

	TerminateThread(this->threadHandle, NULL);
	CloseHandle(this->threadHandle);
}



HANDLE addonThread::Start(LPTHREAD_START_ROUTINE function, LPVOID param)
{
	DWORD tmpH = NULL;
	HANDLE threadH = CreateThread(NULL, NULL, function, param, NULL, &tmpH);

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

	this->mutexHandle = this->Create(std::string("samp-addon"));
}



addonMutex::~addonMutex()
{
	addonDebug("Mutex deconstructor called");
}



HANDLE addonMutex::Create(std::string mutexcode)
{
	HANDLE mutexH = CreateMutex(NULL, FALSE, (LPCWSTR)(mutexcode.c_str()));

	addonDebug("Mutex with handle %i successfuly created", mutexH);

	return mutexH;
}



void addonMutex::Delete()
{
	addonDebug("Mutex with handle %i was deleted", this->mutexHandle);

	this->unLock();
	CloseHandle(this->mutexHandle);

	this->~addonMutex();
}



void addonMutex::Lock()
{
	WaitForSingleObject(this->mutexHandle, INFINITE);
}



void addonMutex::unLock()
{
	ReleaseMutex(this->mutexHandle);
}