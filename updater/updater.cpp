#pragma once



#include "updater.h"





int main()
{
	bool install = false;
	HKEY rKey;
	char rString[512] = {NULL};
	DWORD rLen = sizeof(rString);

	RegOpenKeyEx(HKEY_CURRENT_USER, "Software\\SAMP", NULL, KEY_QUERY_VALUE, &rKey);
	RegQueryValueEx(rKey, "gta_sa_exe", NULL, NULL, (BYTE *)rString, &rLen);
	RegCloseKey(rKey);

	std::string cmdline(GetCommandLine());
	//cmdline.erase(0, (cmdline.find("gta_sa.exe") + 11));
	cmdline.erase(0, (cmdline.find("updater.exe") + 13));

	std::string path(rString);
	path.erase(path.find("gta_sa.exe"), INFINITE);

	SetCurrentDirectory(path.c_str());

	boost::filesystem::path changelog(path + "addon_changelog.log");
	boost::filesystem::path tmpfile(path + "d3d9.tmp");
	boost::filesystem::path dllfile(path + "d3d9.dll");

	install = ((cmdline.find("-n") == std::string::npos) && !boost::filesystem::exists(dllfile));

	printf("SAMP-Addon %s was started\n", (install) ? ("installer") : ("updater"));
	printf("SA-MP installation folder: %s\n", path.c_str());
	printf("Parameters passed: %s\n", cmdline.c_str());

	if(!install)
	{
		printf("Terminating game instance...\n");
		WinExec("taskkill /F /IM gta_sa.exe", SW_SHOW);
		Sleep(5000);
	}

	printf("Downloading package...\n");

	path += "d3d9.tmp";
	HRESULT res = URLDownloadToFile(NULL, "https://www.dropbox.com/s/ga4sg6r9gdc6uva/d3d9.tmp", path.c_str(), NULL, NULL);
	path.erase(path.find("d3d9."), INFINITE);

	if(res == S_OK)
	{
		printf("Downloaded update package (%i bytes)\n", boost::filesystem::file_size(tmpfile));
	}
	else
	{
		printf("Error while downloading update package: %i (Error code: %i)\n", res, GetLastError());

		Sleep(5000);
		exit(EXIT_SUCCESS);
	}

	if((install || boost::filesystem::exists(dllfile)) && boost::filesystem::exists(tmpfile))
	{
		boost::system::error_code error;

		if(!install)
		{
			printf("Removing old d3d9.dll...\n");

			boost::filesystem::remove(dllfile, error);

			if (error)
			{
				printf("Cannot remove old d3d9.dll: %s (Error code: %i)\n", error.message().c_str(), error.value());

				Sleep(5000);
				exit(EXIT_SUCCESS);
			}
		}

		printf("Replacing d3d9.tmp to d3d9.dll location...\n");

		boost::filesystem::rename(tmpfile, dllfile, error);

		if(error)
		{
			printf("Cannot rename d3d9.tmp to d3d9.dll: %s (Error code: %i)\n", error.message().c_str(), error.value());

			Sleep(5000);
			exit(EXIT_SUCCESS);
		}
	}

	printf("Downloading changelog file...\n");

	if(boost::filesystem::exists(changelog))
		boost::filesystem::remove(changelog);

	path += "addon_changelog.log";
	res = URLDownloadToFile(NULL, "https://www.dropbox.com/s/m8ltfynnv0bgylz/changes.txt", path.c_str(), NULL, NULL);
	path.erase(path.find("addon_changelog."), INFINITE);

	if(res == S_OK)
	{
		printf("Changes file saved to addon_changelog.log\n");
	}
	else
	{
		printf("Cannot download changelog file: %i (Error code: %i)\n", res, GetLastError());
	}

	if(install)
	{
		printf("SAMP-Addon was installed!\n");

		Sleep(5000);
		exit(EXIT_SUCCESS);
	}

	WinExec("taskkill /F /IM samp.exe", SW_SHOW);

	Sleep(5000);

	printf("Launching samp.exe with parameters: %s\n", cmdline.c_str());

	STARTUPINFO updaterStart;
	PROCESS_INFORMATION updaterStartInfo;

	ZeroMemory(&updaterStart, sizeof(updaterStart));
	updaterStart.cb = sizeof(updaterStart);

	ZeroMemory(&updaterStartInfo, sizeof(updaterStartInfo));
	path += "samp.exe";

	if(!CreateProcess(path.c_str(), (LPSTR)cmdline.c_str(), NULL, NULL, FALSE, DETACHED_PROCESS, NULL, NULL, &updaterStart, &updaterStartInfo))
	{
		printf("Error while creating process samp.exe: %i\n", GetLastError());

		Sleep(5000);
		exit(EXIT_SUCCESS);
	}

	printf("Update completed!\n");

	Sleep(5000);

	return 1;
}