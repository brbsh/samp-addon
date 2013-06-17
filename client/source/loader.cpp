#pragma once

#include "loader.h"





BOOL WINAPI DllMain(HINSTANCE hinstDLL, DWORD fdwReason, LPVOID lpvReserved)
{
	if(fdwReason == DLL_PROCESS_ATTACH)
	{
		remove("SAMP\\addon\\addon.log");
			
		addonDebug("\tDebugging started\n");
		addonDebug("-----------------------------------------------------------------");
		addonDebug("Called asi attach | hinstDLL 0x%x | fdwReason %i | lpvReserved %i", hinstDLL, fdwReason, lpvReserved);
		addonDebug("-----------------------------------------------------------------");

		if(!GetModuleHandleW(L"gta_sa.exe") || !GetModuleHandleW(L"VorbisFile.dll") || !GetModuleHandleW(L"VorbisHooked.dll"))
		{
			addonDebug("Loader attached from unknown process, terminating");

			return false;
		}

		CreateThread(NULL, NULL, (LPTHREAD_START_ROUTINE)addon_load_thread, (void *)hinstDLL, NULL, NULL);
	}
	else if(fdwReason == DLL_PROCESS_DETACH)
	{
		addonDebug("-----------------------------------------------------------------");
		addonDebug("Called asi detach | hinstDLL 0x%x | fdwReason %i | lpvReserved %i", hinstDLL, fdwReason, lpvReserved);
		addonDebug("-----------------------------------------------------------------\n");
		addonDebug("\tDebugging stopped");
	}

	return true;
}



DWORD addon_load_thread(void *lpParam)
{
	int parentDLL = (int)lpParam;
	unsigned long int tick = 0;

	while(!GetModuleHandleW(L"samp.dll"))
	{
		if(++tick > 4294967293)
		{
			addonDebug("Singleplayer loaded, terminating");

			return false;
		}

		Sleep(1);
	}

	HMODULE addon;
	HMODULE audio;

	SetDllDirectoryW(L"SAMP\\addon");
	addon = LoadLibraryW(L"addon.dll");

	if(!addon)
	{
		addonDebug("Cannot launch SAMP\\addon\\addon.dll");

		return false;
	}

	addonLoader addonLoad = (addonLoader)GetProcAddress(addon, "addon_start");

	if(!addonLoad)
	{
		addonDebug("Cannot export main loader function");

		return false;
	}

	addonLoad();

	Sleep(5000);



	if(GetModuleHandleW(L"audio.dll"))
	{
		addonDebug("Audio plugin already loaded");

		return false;
	}

	SetDllDirectoryW(L"SAMP\\addon\\audio");
	audio = LoadLibraryW(L"audio.dll");

	if(!audio)
	{
		addonDebug("Audio plugin wasn't found, processing without it...");

		return false;
	}

	addonLoader audioLoad = (addonLoader)GetProcAddress(audio, "startPlugin");

	if(!audioLoad)
	{
		addonDebug("Invalid audio plugin (v0.5 R2 needed)");

		return false;
	}

	/*
	if(GetModuleHandleW(L"bass.dll"))
	{
		while(BASS_GetDevice() == -1)
		{
			Sleep(1000);
		}
	}
	*/

	audioLoad();

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