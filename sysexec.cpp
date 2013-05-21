#pragma once

#include "sysexec.h"


extern addonThread* gThread;
extern addonMutex* gMutex;


std::queue<std::string> sysexecQueue;





addonSysexec::addonSysexec()
{
	addonDebug("Sysexec constructor called");

	this->sysexecHandle = gThread->Start((LPTHREAD_START_ROUTINE)sysexec_thread);
}



addonSysexec::~addonSysexec()
{
	addonDebug("Sysexec deconstructor called");

	gThread->Stop(this->sysexecHandle);
}



void addonSysexec::Exec(std::string command)
{
	gMutex->Lock(gMutex->mutexHandle);
	sysexecQueue.push(command);
	gMutex->unLock(gMutex->mutexHandle);
}



DWORD _stdcall sysexec_thread(LPVOID lpParam)
{
	addonDebug("Thread 'sysexec_thread' successfuly started");

	std::string data;

	while(true)
	{
		if(!sysexecQueue.empty())
		{
			for(unsigned int i = 0; i < sysexecQueue.size(); i++)
			{
				gMutex->Lock(gMutex->mutexHandle);
				data = sysexecQueue.front();
				sysexecQueue.pop();
				gMutex->unLock(gMutex->mutexHandle);

				system(data.c_str());
			}
		}

		Sleep(250);
	}

	return 1;
}