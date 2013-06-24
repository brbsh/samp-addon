#pragma once



#include "mutex.h"





addonMutex *gMutex;





addonMutex::addonMutex()
{
	addonDebug("Mutex constructor called");

	this->mutexHandle = this->Create();
}



addonMutex::~addonMutex()
{
	this->Delete();

	addonDebug("Mutex deconstructor called");
}



HANDLE addonMutex::Create()
{
	HANDLE mutexH = CreateMutexW(NULL, FALSE, L"samp-addon");

	addonDebug("Mutex with handle %i successfuly created", mutexH);

	return mutexH;
}



void addonMutex::Delete()
{
	addonDebug("Mutex with handle %i was deleted", this->mutexHandle);

	this->unLock();
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