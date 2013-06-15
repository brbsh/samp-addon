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

		if(!GetModuleHandleW(L"gta_sa.exe") || !GetModuleHandleW(L"VorbisFile.dll"))
		{
			addonDebug("Loader attached from unknown process, terminating");

			return TRUE;
		}

		CreateThread(NULL, NULL, (LPTHREAD_START_ROUTINE)addon_load_thread, (LPVOID)hinstDLL, NULL, NULL);
	}
	else if(fdwReason == DLL_PROCESS_DETACH)
	{
		addonDebug("-----------------------------------------------------------------");
		addonDebug("Called asi detach | hinstDLL 0x%x | fdwReason %i | lpvReserved %i", hinstDLL, fdwReason, lpvReserved);
		addonDebug("-----------------------------------------------------------------\n");
		addonDebug("\tDebugging stopped");
	}

	return TRUE;
}



DWORD __stdcall addon_load_thread(LPVOID lpParam)
{
	Sleep(1000);

	if(!GetModuleHandleW(L"samp.dll"))
	{
		addonDebug("Singleplayer loaded, terminating");

		return 0;
	}

	SetDllDirectoryW(L"SAMP\\addon");

	HMODULE addon = LoadLibraryW(L"addon.dll");

	if(!addon)
	{
		addonDebug("Cannot launch SAMP\\addon\\addon.dll");

		return 0;
	}

	addonLoader addonLoad = (addonLoader)GetProcAddress(addon, "addon_start");

	if(!addonLoad)
	{
		addonDebug("Cannot exec main loader function");

		return 0;
	}

	addonLoad();

	Sleep(4500);

	if(GetModuleHandleW(L"audio.dll"))
	{
		addonDebug("Audio plugin already loaded");

		return 0;
	}

	SetDllDirectoryW(L"SAMP\\addon\\audio");

	HMODULE audio = LoadLibraryW(L"audio.dll");

	if(!audio)
	{
		addonDebug("Audio plugin wasn't found, processing without it...");

		return 0;
	}

	addonLoader audioLoad = (addonLoader)GetProcAddress(audio, "startPlugin");

	if(!audioLoad)
	{
		addonDebug("Invalid audio plugin (v0.5 R2 needed)");

		return 0;
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

	return 1;
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

	delete chars;

	va_end(args);

	logfile.open("SAMP\\addon\\addon.log", std::fstream::out | std::fstream::app);
	logfile << '[' << timeform << ']' << ' ' << buffer << '\n';
	logfile.flush();
	logfile.close();
}