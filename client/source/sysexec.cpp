#pragma once

#include "sysexec.h"



extern addonThread *gThread;
extern addonMutex *gMutex;


std::queue<std::string> sysexecQueue;





addonSysexec::addonSysexec()
{
	addonDebug("Sysexec constructor called");

	this->sysexecHandle = gThread->Start((LPTHREAD_START_ROUTINE)sysexec_thread, NULL);
}



addonSysexec::~addonSysexec()
{
	addonDebug("Sysexec deconstructor called");

	gThread->Stop(this->sysexecHandle);
}



void addonSysexec::Exec(std::string command)
{
	gMutex->Lock();
	sysexecQueue.push(command);
	gMutex->unLock();
}



DWORD sysexec_thread(void *lpParam)
{
	addonDebug("Thread 'sysexec_thread' successfuly started");

	std::string data;

	do
	{
		if(!sysexecQueue.empty())
		{
			for(unsigned int i = 0; i < sysexecQueue.size(); i++)
			{
				gMutex->Lock();
				data = sysexecQueue.front();
				sysexecQueue.pop();
				gMutex->unLock();

				system(data.c_str());
			}
		}

		Sleep(250);
	}
	while(true);

	return true;
}