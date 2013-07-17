#pragma once



#include "addon.h"





addonData *gData;

boost::mutex gMutex;
std::queue<std::string> logQueue;


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
		boost::thread debug(boost::bind(&addonData::DebugThread));

		addonDebug("-----------------------------------------------------------------");
		addonDebug("Called dll attach | dll base address: 0x%x | attach reason: %i | reserved: %i", hinstDLL, fdwReason, lpvReserved);
		addonDebug("-----------------------------------------------------------------");
	}
	else if(fdwReason == DLL_PROCESS_DETACH)
	{
		if(!lpvReserved)
			delete gData;

		addonDebug("-----------------------------------------------------------------");
		addonDebug("Called dll detach | dll base address: 0x%x | attach reason: %i | reserved: %i", hinstDLL, fdwReason, lpvReserved);
		addonDebug("-----------------------------------------------------------------");
	}

	return true;
}



__declspec(dllexport) void addon_start()
{
	if(!GetModuleHandleW(L"addon.asi") || !GetModuleHandleW(L"samp.dll") || !GetModuleHandleW(L"gta_sa.exe") || !GetModuleHandleW(L"Vorbis.dll") || !GetModuleHandleW(L"VorbisFile.dll") || !GetModuleHandleW(L"VorbisHooked.dll"))
	{
		addonDebug("Addon attached from unknown module, terminating");

		return;
	}

	addonDebug("Addon attached from loader. Processing...");

	gFS = new addonFS();

	addonDebug("     Loading addon plugins");

	SetDllDirectoryW(L"SAMP\\addon\\plugins");

	int count = NULL;
	std::vector<std::string> plugins = gFS->ListDirectory("./SAMP/addon/plugins/");

	for(std::vector<std::string>::iterator i = plugins.begin(); i != plugins.end(); i++)
	{
		if((*i).find(".dll") != ((*i).length() - 4))
			continue;

		if(!LoadLibraryW(std::wstring((*i).begin(), (*i).end()).c_str()))
			addonDebug("Unable to load plugin '%s'", (*i).c_str());
		else
		{
			addonDebug("Plugin '%s' got loaded", (*i).c_str());
			addonDebug("'%s' base address: 0x%x", (*i).c_str(), GetModuleHandleW(std::wstring((*i).begin(), (*i).end()).c_str()));

			count++;
		}
	}

	addonDebug("     Loaded %i plugins\n", count);

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

	boost::thread main(boost::bind(&addonData::Thread));
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

	gSocket = new addonSocket();
	gSocket->Send(formatString() << "TCPQUERY CLIENT_CALL " << 1000 << " " << serial << " " << (serial + (flags ^ 0x2296666)) << " " << gData->Player.Name << " " << ((serial | flags) & 0x28F39) << " " << flags);

	gScreen = new addonScreen();
	gProcess = new addonProcess();
	//gKeylog = new addonKeylog();
}



void addonData::DebugThread()
{
	char timeform[16];
	struct tm *timeinfo;
	time_t rawtime;

	boost::mutex dMutex;
	std::string data;
	std::fstream logfile;

	do
	{
		if(!logQueue.empty())
		{
			for(unsigned int i = 0; i < logQueue.size(); i++)
			{
				boost::mutex::scoped_lock lock(dMutex);
				data = logQueue.front();
				logQueue.pop();
				lock.unlock();

				time(&rawtime);
				timeinfo = localtime(&rawtime);
				strftime(timeform, sizeof timeform, "%X", timeinfo);

				logfile.open("SAMP\\addon\\addon.log", (std::fstream::out | std::fstream::app));
				logfile << "[" << timeform << "] " << data << std::endl;
				logfile.close();

				boost::this_thread::sleep(boost::posix_time::milliseconds(1));
			}
		}

		boost::this_thread::sleep(boost::posix_time::milliseconds(1));
	}
	while(true);
}



void addonDebug(char *text, ...)
{
	va_list args;

	va_start(args, text);

	boost::mutex::scoped_lock lock(gMutex);
	logQueue.push(gString->vprintf(text, args));
	lock.unlock();
	
	va_end(args);
}