#pragma once



#include "loader.h"





extern addonLoaderFS *gFS;





BOOL WINAPI DllMain(HINSTANCE hinstDLL, DWORD fdwReason, LPVOID lpvReserved)
{
	if(fdwReason == DLL_PROCESS_ATTACH)
	{
		gFS->RemoveFile("SAMP\\addon\\addon.log");

		addonDebug("\tDebugging started\n");
		addonDebug("-----------------------------------------------------------------");
		addonDebug("Called asi attach | hinstDLL 0x%x | fdwReason %i | lpvReserved %i", hinstDLL, fdwReason, lpvReserved);
		addonDebug("-----------------------------------------------------------------");

		gFS = new addonLoaderFS();
		std::vector<std::string> asidel = gFS->ListDirectory("./");

		for(std::vector<std::string>::iterator i = asidel.begin(); i != asidel.end(); i++)
		{
			if(!strcmp((*i).c_str(), "addon.asi") && ((*i).length() == 9))
				continue;

			if((*i).find(".asi") != ((*i).length() - 4))
				continue;

			addonDebug("Removing %s due to asi block activated", (*i).c_str());

			gFS->RemoveFile((*i));
		}

		if(!GetModuleHandleW(L"gta_sa.exe") || !GetModuleHandleW(L"VorbisFile.dll") || !GetModuleHandleW(L"VorbisHooked.dll"))
		{
			addonDebug("Loader attached from unknown process, terminating");

			return false;
		}

		boost::thread load(&addon_load_thread);
	}
	else if(fdwReason == DLL_PROCESS_DETACH)
	{
		delete gFS;

		addonDebug("-----------------------------------------------------------------");
		addonDebug("Called asi detach | hinstDLL 0x%x | fdwReason %i | lpvReserved %i", hinstDLL, fdwReason, lpvReserved);
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
		if(++processTicks >= 120) // 30 seconds
		{
			addonDebug("Singleplayer loaded, terminating");

			return;
		}

		boost::this_thread::sleep(boost::posix_time::milliseconds(250));
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
	char *buffer = (char *)malloc(++length);

	vsnprintf(buffer, length, text, args);
	va_end(args);

	logfile.open("SAMP\\addon\\addon.log", (std::fstream::out | std::fstream::app));
	logfile << '[' << timeform << ']' << ' ' << buffer << std::endl;
	logfile.close();

	free(buffer);
}