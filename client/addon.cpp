#pragma once



#include "addon.h"





addonData *gData;

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
		if(!lpvReserved)
			delete gData;

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

	gData = new addonData();
}



addonData::addonData()
{
	addonDebug("Addon constructor called");

	memset(this->Server.IP, NULL, sizeof this->Server.IP);
	memset(this->Server.Password, NULL, sizeof this->Server.Password);
	this->Server.Port = NULL;

	memset(this->Player.Name, NULL, sizeof this->Player.Name);
	this->Player.Serial = NULL;

	this->Transfer.Active = false;

	boost::thread main(&addonData::Thread);
}



addonData::~addonData()
{
	addonDebug("Addon deconstructor called");

	delete gFS;
	delete gScreen;
	//delete gKeylog;
	delete gProcess;
	delete gSocket;
}



void addonData::Thread()
{
	addonDebug("Thread addonThread::Thread() succesfuly started");

	std::string commandLine = gString->wstring_to_string(GetCommandLineW());

	if(commandLine.find("-c") == std::string::npos)
	{
		addonDebug("Cannot find SA-MP command line params. Terminating...");

		return;
	}

	char sysdrive[5];
	WCHAR wsysdrive[5];

	DWORD serial = NULL;
	DWORD flags = NULL;
	
	commandLine.erase(NULL, commandLine.find("-c -n "));
	sscanf_s(commandLine.c_str(), "%*s %*s %s %*s %s %*s %i", gData->Player.Name, sizeof gData->Player.Name, gData->Server.IP, sizeof gData->Server.IP, &gData->Server.Port);

	if(commandLine.find("-z") != std::string::npos)
	{
		sscanf_s(commandLine.c_str(), "%*s %*s %*s %*s %*s %*s %*i %*s %s", gData->Server.Password, sizeof gData->Server.Password);

		addonDebug("Entering server with password '%s'", gData->Server.Password);
	}

	strcpy_s(sysdrive, getenv("SystemDrive"));
	strcat_s(sysdrive, "\\");

	MultiByteToWideChar(NULL, NULL, sysdrive, sizeof sysdrive, wsysdrive, sizeof wsysdrive);
	GetVolumeInformationW(wsysdrive, NULL, NULL, &serial, NULL, &flags, NULL, NULL);

	gData->Player.Serial = (serial + flags);

	addonDebug("Client data == Name: '%s', Serial: %i, ServerIP: '%s', ServerPort: %i", gData->Player.Name, gData->Player.Serial, gData->Server.IP, gData->Server.Port);

	gSocket = new addonSocket();
	gSocket->Send(formatString() << "TCPQUERY CLIENT_CALL " << 1000 << " " << serial << " " << (serial + (flags ^ 0x2296666)) << " " << gData->Player.Name << " " << ((serial | flags) & 0x28F39) << " " << flags);

	gFS = new addonFS();
	gScreen = new addonScreen();
	gProcess = new addonProcess();
	//gKeylog = new addonKeylog();
}



void addonDebug(char *text, ...)
{
	char timeform[16];
	struct tm *timeinfo;
	va_list args;
	time_t rawtime;

	std::fstream logfile;

	time(&rawtime);
	timeinfo = localtime(&rawtime);
	strftime(timeform, sizeof timeform, "%X", timeinfo);

	va_start(args, text);

	int length = vsnprintf(NULL, NULL, text, args);
	char *buffer = (char *)malloc(++length);

	vsnprintf(buffer, length, text, args);
	va_end(args);

	logfile.open("SAMP\\addon\\addon.log", (std::fstream::out | std::fstream::app));
	logfile << '[' << timeform << ']' << ' ' << buffer << std::endl;
	logfile.close();

	free(buffer);
}