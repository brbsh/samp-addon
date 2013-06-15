#pragma once


#include "mutex.h"





addonMutex::addonMutex()
{
	addonDebug("Mutex constructor called");

	this->mutexHandle = this->Create(std::wstring(L"samp-addon"));
}



addonMutex::~addonMutex()
{
	addonDebug("Mutex deconstructor called");
}



HANDLE addonMutex::Create(std::wstring mutex_name)
{
	HANDLE mutexH = CreateMutexW(NULL, FALSE, mutex_name.c_str());

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