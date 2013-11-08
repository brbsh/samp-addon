#pragma once



#include "updater.h"





int main()
{
	boost::filesystem::path changelog(".\\addon_changelog.log");
	boost::filesystem::path tmpfile(".\\d3d9.tmp");
	boost::filesystem::path dllfile(".\\d3d9.dll");

	printf("SAMP-Addon updater was started\n");
	printf("Parameters passed: %s", GetCommandLine());
	printf("Downloading update...\n");

	HRESULT res = URLDownloadToFile(NULL, "http://addon.bjiadokc.ru/client/d3d9.tmp", ".\\d3d9.tmp", NULL, NULL);

	if(res == S_OK)
	{
		printf("Downloaded update package (%i bytes)\n", boost::filesystem::file_size(tmpfile));
	}
	else
	{
		printf("Error while downloading update package: %i (Error code: %i)\n", res, GetLastError());

		system("pause");
	}

	if(boost::filesystem::exists(tmpfile) && boost::filesystem::exists(dllfile))
	{
		boost::system::error_code error;

		printf("Removing old d3d9.dll...\n");

		boost::filesystem::remove(dllfile, error);

		if(error)
		{
			printf("Cannot remove old d3d9.dll: %s (Error code: %i)\n", error.message().c_str(), error.value());

			system("pause");
		}

		printf("Replacing d3d9.tmp to d3d9.dll location...\n");

		boost::filesystem::rename(tmpfile, dllfile, error);

		if (error)
		{
			printf("Cannot rename d3d9.tmp to d3d9.dll: %s (Error code: %i)\n", error.message().c_str(), error.value());

			system("pause");
		}
	}

	printf("Downloading changelog file...\n");

	if(boost::filesystem::exists(changelog))
		boost::filesystem::remove(changelog);

	res = URLDownloadToFile(NULL, "http://addon.bjiadokc.ru/client/changes.txt", ".\\addon_changelog.log", NULL, NULL);

	if(res == S_OK)
	{
		printf("Changes file saved to addon_changelog.log\n");
	}
	else
	{
		printf("Cannot download changelog file: %i (Error code: %i)\n", res, GetLastError());
	}

	WinExec("taskkill /F /IM samp.exe", SW_SHOW);

	Sleep(5000);

	printf("Launching samp.exe with parameters: %s\n", GetCommandLine());

	STARTUPINFO updaterStart;
	PROCESS_INFORMATION updaterStartInfo;

	ZeroMemory(&updaterStart, sizeof(updaterStart));
	updaterStart.cb = sizeof(updaterStart);

	ZeroMemory(&updaterStartInfo, sizeof(updaterStartInfo));

	if (CreateProcess("samp.exe", GetCommandLine(), NULL, NULL, FALSE, DETACHED_PROCESS, NULL, NULL, &updaterStart, &updaterStartInfo))
	{

	}
	else
	{
		printf("Error while creating process samp.exe: %i\n", GetLastError());
	}

	printf("Update completed!\n");

	system("exit");

	return 1;
}