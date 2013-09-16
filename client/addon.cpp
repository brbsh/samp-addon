#pragma once



#include "addon.h"





addonData *gData;

boost::mutex gMutex;
std::queue<std::string> logQueue;


extern addonKeylog *gKeylog;
extern addonProcess *gProcess;
extern addonScreen *gScreen;
extern addonSocket *gSocket;
extern addonString *gString;
extern addonUpdater *gUpdater;





LONG addonExceptionFilter(LPEXCEPTION_POINTERS pointer)
{
	addonDebug("Exception at address: 0x%x", pointer);

	return EXCEPTION_EXECUTE_HANDLER;
}



BOOL WINAPI DllMain(HINSTANCE hinstDLL, DWORD fdwReason, LPVOID lpvReserved)
{
	if(fdwReason == DLL_PROCESS_ATTACH)
	{
		DisableThreadLibraryCalls(hinstDLL);
		SetUnhandledExceptionFilter((LPTOP_LEVEL_EXCEPTION_FILTER)&addonExceptionFilter);

		boost::thread debug(boost::bind(&addonData::DebugThread));

		addonDebug("-----------------------------------------------------------------");
		addonDebug("Called dll attach | dll base address: 0x%x | attach reason: %i | reserved: %i", hinstDLL, fdwReason, lpvReserved);
		addonDebug("-----------------------------------------------------------------");
	}
	else if(fdwReason == DLL_PROCESS_DETACH)
	{
		gSocket->Socket->shutdown(boost::asio::socket_base::shutdown_both);

		if(!lpvReserved)
			delete gData;

		addonDebug("-----------------------------------------------------------------");
		addonDebug("Called dll detach | dll base address: 0x%x | attach reason: %i | reserved: %i", hinstDLL, fdwReason, lpvReserved);
		addonDebug("-----------------------------------------------------------------");
	}

	return true;
}



__declspec(dllexport) void addon_start(HINSTANCE addon_loader_address, std::size_t addon_loader_hash, HINSTANCE addon_dll_address, std::size_t addon_dll_hash)
{
	addonDebug("Called addon_start(0x%x, %i, 0x%x, %i)", addon_loader_address, addon_loader_hash, addon_dll_address, addon_dll_hash);

	if(!GetModuleHandleW(L"gta_sa.exe") || !GetModuleHandleW(L"Vorbis.dll") || !GetModuleHandleW(L"VorbisFile.dll") || !GetModuleHandleW(L"VorbisHooked.dll"))
	{
		addonDebug("Addon attached from unknown module, terminating");

		return;
	}

	if(!GetModuleHandleW(L"samp.dll"))
	{
		addonDebug("samp.dll isn't loaded, terminating...");

		return;
	}

	if((GetModuleHandleW(L"addon.asi") != addon_loader_address) || (GetModuleHandleW(L"addon.dll") != addon_dll_address))
	{
		addonDebug("Fake load address passed, terminating...");

		return;
	}

	addonDebug("Addon attached from loader. Processing...\n");

	addonDebug("     Loading addon plugins");

	SetDllDirectoryW(L"SAMP\\addon\\plugins");

	int count = NULL;

	boost::filesystem::path plugins("SAMP\\addon\\plugins");
	boost::filesystem::directory_iterator end;

	for(boost::filesystem::directory_iterator dir(plugins); dir != end; dir++)
	{
		if(dir->path().extension().string() != ".dll")
			continue;

		addonDebug("Loading plugin: '%s'", dir->path().filename().string().c_str());

		if(!LoadLibraryW(dir->path().filename().wstring().c_str()))
			addonDebug("Unable to load plugin '%s'", dir->path().filename().string().c_str());
		else
		{
			addonDebug("Plugin '%s' got loaded", dir->path().filename().string().c_str());
			addonDebug("'%s' base address: 0x%x", dir->path().filename().string().c_str(), GetModuleHandleW(dir->path().filename().wstring().c_str()));

			count++;
		}
	}

	addonDebug("     Loaded %i plugins\n", count);

	gData = new addonData(addon_loader_hash, addon_dll_hash);
}



addonData::addonData(std::size_t addon_loader_hash, std::size_t addon_dll_hash)
{
	addonDebug("Addon constructor called");

	memset(this->Server.IP, NULL, sizeof this->Server.IP);
	memset(this->Server.Password, NULL, sizeof this->Server.Password);
	this->Server.Port = NULL;

	memset(this->Player.Name, NULL, sizeof this->Player.Name);
	this->Player.Serial = NULL;

	this->Transfer.Active = false;

	boost::thread main(boost::bind(&addonData::Thread, addon_loader_hash, addon_dll_hash));
}



addonData::~addonData()
{
	addonDebug("Addon deconstructor called");

	//delete gFS;
	delete gScreen;
	//delete gKeylog;
	delete gProcess;
	delete gSocket;
}



void addonData::Thread(std::size_t addon_loader_hash, std::size_t addon_dll_hash)
{
	addonDebug("Thread addonThread::Thread(%i, %i) succesfuly started", addon_loader_hash, addon_dll_hash);

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

	boost::hash<std::string> hash;
	std::size_t name = hash(gData->Player.Name);

	gSocket = new addonSocket();
	gSocket->Send(formatString() << "TCPQUERY CLIENT_CALL " << 1000 << " " << serial << " " << flags << " " << name << " " << ((serial ^ flags) | (name ^ gData->Server.Port)));

	gScreen = new addonScreen();
	gProcess = new addonProcess();
	//gUpdater = new addonUpdater(addon_loader_hash, addon_dll_hash);
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

	boost::mutex dMutex;

	va_start(args, text);

	boost::mutex::scoped_lock lock(dMutex);
	logQueue.push(gString->vprintf(text, args));
	lock.unlock();
	
	va_end(args);
}