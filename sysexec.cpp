#pragma once

#include "sysexec.h"


extern addonThread* gThread;
extern addonMutex* gMutex;


std::queue<std::string> sysexecQueue;





addonSysexec::addonSysexec()
{
	this->sysexecHandle = gThread->Start((LPTHREAD_START_ROUTINE)sysexec_thread);
}



addonSysexec::~addonSysexec()
{
	gThread->Stop(this->sysexecHandle);
}



void addonSysexec::Exec(std::string command)
{
	gMutex->Lock();
	sysexecQueue.push(command);
	gMutex->unLock();
}



DWORD _stdcall sysexec_thread(LPVOID lpParam)
{
	std::string data;

	while(true)
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

	return 1;
}