#pragma once

#include "mouselog.h"



extern addonThread *gThread;
extern addonMutex *gMutex;
extern addonSocket *gSocket;
extern addonMouselog *gMouselog;
extern std::queue<std::string> send_to;





addonMouselog::addonMouselog()
{
	addonDebug("Mouselog constructor called");

	this->active = true;

	this->mouselogHandle = gThread->Start((LPTHREAD_START_ROUTINE)mouselog_thread, NULL);
}



addonMouselog::~addonMouselog()
{
	addonDebug("Mouselog deconstructor called");

	this->active = false;

	gThread->Stop(this->mouselogHandle);
}



DWORD __stdcall mouselog_thread(LPVOID lpParam)
{
	addonDebug("Thread 'mouselog_thread' successfuly started");

	POINT currentPoint, prevPoint;
	std::stringstream format;

	while(gMouselog->active)
	{
		GetCursorPos(&currentPoint);

		if((prevPoint.x != currentPoint.x) || (prevPoint.y != currentPoint.y))
		{
			format << "TCPQUERY" << '>' << "CLIENT_CALL" << '>' << 1235 << '>' << currentPoint.x << '_' << currentPoint.y; // "TCPQUERY>CLIENT_CALL>1235>%i_%i   ---   Sends mouse cursor pos (array) | %i_%i = x_y

			gSocket->Send(format.str());
			
			format.clear();
			prevPoint = currentPoint;
		}

		Sleep(250);
	}

	return true;
}
