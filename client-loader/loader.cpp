#pragma once



#include "loader.h"





extern addonLoaderFS *gFS;





BOOL WINAPI DllMain(HINSTANCE hinstDLL, DWORD fdwReason, LPVOID lpvReserved)
{
	if(fdwReason == DLL_PROCESS_ATTACH)
	{
		if(gFS->FileExist("SAMP\\addon\\addon.log"))
			gFS->RemoveFile("SAMP\\addon\\addon.log");

		addonDebug("\tDebugging started\n");
		addonDebug("-----------------------------------------------------------------");
		addonDebug("Called asi attach | asi base address: 0x%x | attach reason: %i | reserved: %i", hinstDLL, fdwReason, lpvReserved);
		addonDebug("-----------------------------------------------------------------");

		if(!gFS->FileExist("addon.asi") || (GetModuleHandleW(L"addon.asi") != hinstDLL))
		{
			addonDebug("Addon loader must be named as 'addon.asi'. Please, rename it");

			return false;
		}

		gFS = new addonLoaderFS();
		std::vector<std::string> asidel = gFS->ListDirectory("./");

		for(std::vector<std::string>::iterator i = asidel.begin(); i != asidel.end(); i++)
		{
			if(!strcmp((*i).c_str(), "addon.asi") && ((*i).length() == 9))
				continue;

			if((*i).find(".asi") != ((*i).length() - 4))
				continue;

			addonDebug("Removing '%s' due to asi block activated", (*i).c_str());

			gFS->RemoveFile((*i));
		}

		if(!GetModuleHandleW(L"gta_sa.exe") || !GetModuleHandleW(L"Vorbis.dll") || !GetModuleHandleW(L"VorbisFile.dll") || !GetModuleHandleW(L"VorbisHooked.dll"))
		{
			addonDebug("Loader attached from unknown process, terminating");

			return false;
		}

		addonDebug("Game process base address: 0x%x", GetModuleHandleW(L"gta_sa.exe"));
		addonDebug("Vorbis library base address: 0x%x", GetModuleHandleW(L"Vorbis.dll"));
		addonDebug("VorbisFile hook base address: 0x%x", GetModuleHandleW(L"VorbisFile.dll"));
		addonDebug("VorbisFile library base address: 0x%x", GetModuleHandleW(L"VorbisHooked.dll"));

		boost::thread load(boost::bind(&addon_load_thread));
	}
	else if(fdwReason == DLL_PROCESS_DETACH)
	{
		delete gFS;

		addonDebug("-----------------------------------------------------------------");
		addonDebug("Called asi detach | asi base address: 0x%x | detach reason: %i | reserved: %i", hinstDLL, fdwReason, lpvReserved);
		addonDebug("-----------------------------------------------------------------\n");
		addonDebug("\tDebugging stopped");
	}

	return true;
}



void addon_load_thread()
{
	int processTicks = NULL;

	while(!GetModuleHandleW(L"samp.dll"))
	{
		if(++processTicks >= 30) // 30 seconds
		{
			addonDebug("Singleplayer loaded, terminating");

			return;
		}

		boost::this_thread::sleep(boost::posix_time::seconds(1));
	}

	SetDllDirectoryW(L"SAMP\\addon");
	HMODULE addon = LoadLibraryW(L"addon.dll");

	if(!addon)
	{
		addonDebug("Cannot launch SAMP\\addon\\addon.dll");

		return;
	}

	addonLoader addonLoad = (addonLoader)GetProcAddress(addon, "addon_start");

	if(!addonLoad)
	{
		addonDebug("Cannot export main loader function");

		return;
	}

	addonLoad();
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
	char *buffer = new char[++length];

	vsnprintf(buffer, length, text, args);
	va_end(args);

	logfile.open("SAMP\\addon\\addon.log", (std::fstream::out | std::fstream::app));
	logfile << "[" << timeform << "] " << buffer << std::endl;
	logfile.close();

	delete[] buffer;
}