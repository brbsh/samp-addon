#pragma once



#include "updater.h"





bool IsProcessRunning(const char *const processName)
{
	HANDLE hSnapshot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, NULL);

	PROCESSENTRY32 pe;
	pe.dwSize = sizeof(PROCESSENTRY32);
	Process32First(hSnapshot, &pe);

	while(true) 
	{
		if(!strcmp(pe.szExeFile, processName)) 
			return true;

		if(!Process32Next(hSnapshot, &pe)) 
			return false;
	}
}



std::size_t crc32_file(std::string filename)
{
	boost::crc_32_type result;
	std::ifstream i;

	i.open(filename, std::fstream::binary);
		
	do
	{
		char block[2048];
		
		i.read(block, sizeof block);
		result.process_bytes(block, i.gcount());
	}
	while(i);

	i.close();

	return result.checksum();
}



int main()
{
	HKEY rKey;
	char rString[512] = {NULL};
	DWORD rLen = sizeof(rString);

	RegOpenKeyEx(HKEY_CURRENT_USER, "Software\\SAMP", NULL, KEY_QUERY_VALUE, &rKey);
	RegQueryValueEx(rKey, "gta_sa_exe", NULL, NULL, (BYTE *)rString, &rLen);
	RegCloseKey(rKey);

	std::string cmdline(GetCommandLine());

	if(!cmdline.length())
	{
		printf("SAMP-Addon: NULL passed to command line, terminaing...");

		system("pause");
		exit(EXIT_SUCCESS);
	}

	std::string path(rString);
	path.erase(path.find("gta_sa.exe"), INFINITE);

	if(!path.length())
	{
		printf("SAMP-Addon: please (re)install GTA:SA & SA:MP first!");

		system("pause");
		exit(EXIT_SUCCESS);
	}

	boost::filesystem::path changelog(".\\addon_changelog.txt");
	boost::filesystem::path dllfile(".\\d3d9.dll");
	boost::filesystem::path tmpfile(".\\d3d9.tmp");

	if(cmdline.find("/createverfile") != std::string::npos)
	{
		boost::filesystem::path version_af(".\\addon_version.txt");
		boost::filesystem::path version_uf(".\\updater_version.txt");

		std::ifstream i;
		std::ofstream f;

		printf("DEBUG: Create ver file. Press any key to proceed\n");
		system("pause");

		if(boost::filesystem::exists(version_af))
			boost::filesystem::remove(version_af);

		if(boost::filesystem::exists(version_uf))
			boost::filesystem::remove(version_uf);
		
		f.open(".\\addon_version.txt", std::ofstream::out);
		f << crc32_file(".\\d3d9.dll");
		f.close();

		f.open(".\\updater_version.txt", std::ofstream::out);
		f << crc32_file(".\\updater.exe");
		f.close();

		exit(EXIT_SUCCESS);
	}

	SetCurrentDirectory(path.c_str());

	if(cmdline.find("/checkforupdates") != std::string::npos)
	{
		printf("SAMP-Addon updater daemon started (version %i)\n", ADDON_UPDATER_VERSION);
		printf("Running flags: %s\n", cmdline.c_str());
		printf("Waiting for GTA:SA process terminate...\n");

		while(IsProcessRunning("gta_sa.exe"))
				Sleep(2500);

		printf("GTA:SA process terminated, processing...\n");

		if(cmdline.find("/removeasiloader") != std::string::npos)
		{
			boost::filesystem::path vorbisfile(path + "VorbisFile.dll");
			boost::filesystem::path vorbishooked(path + "VorbisHooked.dll");


			printf("Found: Remove ASI loader flag, processing...\n");

			if(boost::filesystem::exists(vorbishooked))
			{
				boost::filesystem::remove(vorbisfile);

				boost::filesystem::rename(vorbishooked, vorbisfile);
			}
		}

		HRESULT download = URLDownloadToFile(NULL, "https://raw.githubusercontent.com/BJIADOKC/samp-addon/master/build/client/addon_version.txt", ".\\addon_version.tmp", NULL, NULL);
		std::size_t hashcheck = crc32_file(".\\d3d9.dll");
		std::size_t hashcheck_remote = hashcheck;

		if(download == S_OK)
		{
			std::ifstream f;

			f.open(".\\addon_version.tmp", std::ifstream::in);
			f >> hashcheck_remote;
			f.close();

			boost::filesystem::remove(boost::filesystem::path(".\\addon_version.tmp"));
		}
		else
		{
			printf("Error while retrieving addon version: %i (Error code: %i)", download, GetLastError());
			system("pause");
			exit(EXIT_SUCCESS);
		}

		if(hashcheck != hashcheck_remote)
		{
			printf("Current addon version (%i) was outdated. New version of addon (%i) was found, downloading it", hashcheck, hashcheck_remote);
			printf("Downloading package...\n");

			download = URLDownloadToFile(NULL, "https://raw.githubusercontent.com/BJIADOKC/samp-addon/master/build/client/d3d9.dll", ".\\d3d9.tmp", NULL, NULL);

			if(download == S_OK)
			{
				printf("Downloaded update package\n");
			}
			else
			{
				printf("Error while downloading update package: %i (Error code: %i)\n", download, GetLastError());
				system("pause");
				exit(EXIT_SUCCESS);
			}

			boost::filesystem::remove(dllfile);
			boost::filesystem::rename(tmpfile, dllfile);

			printf("Downloading changelog file...\n");

			if(boost::filesystem::exists(changelog))
				boost::filesystem::remove(changelog);

			download = URLDownloadToFile(NULL, "https://raw.githubusercontent.com/BJIADOKC/samp-addon/master/build/client/changelog.txt", ".\\addon_changelog.txt", NULL, NULL);

			if(download == S_OK)
			{
				printf("Changes file saved to addon_changelog.txt\n");
			}
			else
			{
				printf("Cannot download changelog file: %i (Error code: %i)\n", download, GetLastError());
				system("pause");
				exit(EXIT_SUCCESS);
			}

			exit(EXIT_SUCCESS);
		}

		printf("Addon files is up to date\n");

		exit(EXIT_SUCCESS);
	}

	printf("SAMP-Addon installer was started\n");
	printf("Found GTA:SA at '%s'\n", path.c_str());
	printf("Downloading SAMP-Addon files...\n");

	HRESULT download = URLDownloadToFile(NULL, "https://raw.githubusercontent.com/BJIADOKC/samp-addon/master/build/client/d3d9.dll", ".\\d3d9.tmp", NULL, NULL);

	if(download == S_OK)
	{
		printf("Downloaded update package (%i bytes)\n", boost::filesystem::file_size(tmpfile));
	}
	else
	{
		printf("Error while downloading update package: %i (Error code: %i)\n", download, GetLastError());
		system("pause");
		exit(EXIT_SUCCESS);
	}

	if(boost::filesystem::exists(dllfile))
	{
		if(crc32_file(".\\d3d9.dll") == crc32_file(".\\d3d9.tmp"))
		{
			boost::filesystem::remove(tmpfile);

			printf("Latest version of SAMP-Addon already installed, terminating...\n");
			system("pause");
			exit(EXIT_SUCCESS);
		}

		boost::filesystem::remove(dllfile);
	}

	boost::filesystem::rename(tmpfile, dllfile);

	printf("Downloading changelog file...\n");

	if(boost::filesystem::exists(changelog))
		boost::filesystem::remove(changelog);

	download = URLDownloadToFile(NULL, "https://raw.githubusercontent.com/BJIADOKC/samp-addon/master/build/client/changelog.txt", ".\\addon_changelog.txt", NULL, NULL);

	if(download == S_OK)
	{
		printf("Changes file saved to addon_changelog.txt\n");
	}
	else
	{
		printf("Cannot download changelog file: %i (Error code: %i)\n", download, GetLastError());
		system("pause");
		exit(EXIT_SUCCESS);
	}

	printf("Installation completed!\n");
	system("pause");

	return 1;
}