#pragma once

#include "loader.h"



addonThread* gThread;
addonMutex* gMutex;
extern addonSocket* gSocket;





BOOL APIENTRY DllMain(HMODULE hModule, DWORD ul_reason_for_call, LPVOID lpReserved)
{
	if(ul_reason_for_call == DLL_PROCESS_ATTACH)
	{
		remove("samp-addon.log");
			
		addonDebug("\tDebugging started\n");
		addonDebug("Called asi attach | hModule 0x%x | ul_reason_for_call %i | lpReserved %i", hModule, ul_reason_for_call, lpReserved);

		gThread = new addonThread();
		
		addonDebug("Moved to separated thread");

		gMutex = new addonMutex();
	}
	else if(ul_reason_for_call == DLL_PROCESS_DETACH)
	{
		gSocket->Close();

		delete gThread;
		delete gMutex;

		addonDebug("Called asi detach | hModule 0x%x | ul_reason_for_call %i | lpReserved %i\n", hModule, ul_reason_for_call, lpReserved);
		addonDebug("\tDebugging stopped");
	}

	return true;
}



void addonDebug(char *text, ...)
{
	va_list args;
	std::fstream logfile;
	char timeform[16];
	time_t rawtime;
	time(&rawtime);
	struct tm * timeinfo;
	timeinfo = localtime(&rawtime);
	strftime(timeform, sizeof timeform, "%X", timeinfo);

	va_start(args, text);

	int length = vsnprintf(NULL, 0, text, args);
	char *chars = new char[++length];

	length = vsnprintf(chars, length, text, args);
	std::string buffer(chars);

	delete chars;

	va_end(args);

	logfile.open("samp-addon.log", std::fstream::out | std::fstream::app);

	logfile << '[' << timeform << ']' << ' ' << buffer << '\n';
	logfile.flush();
	logfile.close();
}