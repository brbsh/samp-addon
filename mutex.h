#pragma once


#include "addon.h"





class addonMutex
{

public:

	HANDLE mutexHandle;

	addonMutex();
	~addonMutex();

	HANDLE Create(std::wstring mutex_name);
	void Delete();
	void Lock();
	void unLock();
};