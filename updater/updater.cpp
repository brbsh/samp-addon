#pragma once



#include "updater.h"





std::string strvprintf(const char *format, va_list args)
{
	int length = vsnprintf(NULL, NULL, format, args);
	char *chars = new char[++length];

	vsnprintf(chars, length, format, args);
	std::string result(chars);
	
	delete[] chars;

	return result;
}



void updaterLog(char *format, ...)
{
	char timeform[16];
	struct tm *timeinfo;
	time_t rawtime;
	va_list args;

	std::ofstream file;
	std::string data = strvprintf(format, args);

	va_start(args, format);
	time(&rawtime);
	timeinfo = localtime(&rawtime);
	strftime(timeform, sizeof timeform, "%X", timeinfo);

	file.open(".\\addon_updater_log.txt", (std::ofstream::out | std::ofstream::app));
	file << "[" << timeform << "] " << data << std::endl;
	file.close();
	
	std::cout << data << std::endl;

	va_end(args);
}



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



//INT __stdcall WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, PSTR lpCmdLine, INT nCmdShow)
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
		updaterLog("SAMP-Addon: NULL passed to command line, terminaing...");

		return 1;
	}

	std::string path(rString);
	path.erase(path.find("gta_sa.exe"), INFINITE);

	if(!path.length())
	{
		updaterLog("SAMP-Addon: please (re)install GTA:SA & SA:MP first!");

		return 1;
	}

	boost::filesystem::path updaterlog(".\\addon_updater_log.txt");
	boost::filesystem::path changelog(".\\addon_changelog.txt");
	boost::filesystem::path dllfile(".\\d3d9.dll");
	boost::filesystem::path tmpfile(".\\d3d9.tmp");

	SetCurrentDirectory(path.c_str());

	try
	{
		if(boost::filesystem::exists(updaterlog))
			boost::filesystem::remove(updaterlog);
	}
	catch(boost::filesystem::filesystem_error &err)
	{
		updaterLog("Error while removing file 'addon_updater_log.txt' (What: %s)", err.what());
	}
	
	if(cmdline.find("/uninstall") != std::string::npos)
	{
		updaterLog("Uninstalling SAMP-Addon...");

		try
		{
			boost::filesystem::remove(dllfile);
			boost::filesystem::remove(changelog);
			boost::filesystem::remove(boost::filesystem::path(".\\addon_updater.tmp"));
		}
		catch(boost::filesystem::filesystem_error &err)
		{
			updaterLog("Error while uninstalling SAMP-Addon (What: %s)", err.what());

			return 1;
		}

		return 0;
	}

	if(cmdline.find("/checkforupdates") != std::string::npos)
	{
		updaterLog("SAMP-Addon updater daemon started");
		updaterLog("Running flags: %s", cmdline.c_str());
		updaterLog("Waiting for GTA:SA process terminate...");

		while(IsProcessRunning("gta_sa.exe"))
				Sleep(5000);

		updaterLog("GTA:SA process terminated, processing...");

		if(cmdline.find("/removeasiloader") != std::string::npos)
		{
			boost::filesystem::path vorbisfile(path + "VorbisFile.dll");
			boost::filesystem::path vorbishooked(path + "VorbisHooked.dll");


			updaterLog("Found: Remove ASI loader flag, processing...");

			try
			{
				if(boost::filesystem::exists(vorbishooked))
				{
					boost::filesystem::remove(vorbisfile);
					boost::filesystem::rename(vorbishooked, vorbisfile);
				}
			}
			catch(boost::filesystem::filesystem_error &err)
			{
				updaterLog("Error while removing ASI loader (What: %s)", err.what());

				return 1;
			}
		}

		HRESULT download = URLDownloadToFile(NULL, "https://raw.githubusercontent.com/BJIADOKC/samp-addon/master/build/client/client_version.txt", ".\\addon_version.tmp", NULL, NULL);
		std::size_t hashcheck = crc32_file(".\\d3d9.dll");
		std::size_t hashcheck_remote = hashcheck;

		if(download == S_OK)
		{
			std::ifstream f;

			f.open(".\\addon_version.tmp", std::ifstream::in);
			f >> hashcheck_remote;
			f.close();

			try
			{
				boost::filesystem::remove(boost::filesystem::path(".\\addon_version.tmp"));
			}
			catch(boost::filesystem::filesystem_error &err)
			{
				updaterLog("Error while removing file 'addon_version.tmp' (What: %s)", err.what());

				return 1;
			}
		}
		else
		{
			updaterLog("Error while retrieving addon version: %i (Error code: %i)", download, GetLastError());

			return 1;
		}

		if(hashcheck != hashcheck_remote)
		{
			updaterLog("Current addon version (%i) was outdated. New version of addon (%i) was found, downloading it", hashcheck, hashcheck_remote);
			updaterLog("Downloading package...");

			download = URLDownloadToFile(NULL, "https://raw.githubusercontent.com/BJIADOKC/samp-addon/master/build/client/d3d9.dll", ".\\d3d9.tmp", NULL, NULL);

			if(download == S_OK)
			{
				updaterLog("Downloaded update package");
			}
			else
			{
				updaterLog("Error while downloading update package: %i (Error code: %i)", download, GetLastError());

				return 1;
			}

			try
			{
				boost::filesystem::remove(dllfile);
				boost::filesystem::rename(tmpfile, dllfile);
			}
			catch(boost::filesystem::filesystem_error &err)
			{
				updaterLog("Error while replacing 'd3d9.dll' by 'd3d9.tmp' (What: %s)", err.what());

				return 1;
			}

			updaterLog("Downloading changelog file...");

			try
			{
				if(boost::filesystem::exists(changelog))
					boost::filesystem::remove(changelog);
			}
			catch(boost::filesystem::filesystem_error &err)
			{
				updaterLog("Error while removing file 'addon_changelog.txt' (What: %s)", err.what());

				return 1;
			}

			download = URLDownloadToFile(NULL, "https://raw.githubusercontent.com/BJIADOKC/samp-addon/master/build/client/changelog.txt", ".\\addon_changelog.txt", NULL, NULL);

			if(download == S_OK)
			{
				updaterLog("Changes file saved to addon_changelog.txt");
			}
			else
			{
				updaterLog("Cannot download changelog file: %i (Error code: %i)", download, GetLastError());

				return 1;
			}

			exit(EXIT_SUCCESS);
		}

		updaterLog("Addon files is up to date");

		return 1;
	}

	updaterLog("SAMP-Addon installer was started");
	updaterLog("Found GTA:SA at '%s'", path.c_str());
	updaterLog("Downloading SAMP-Addon files...");

	HRESULT download = URLDownloadToFile(NULL, "https://raw.githubusercontent.com/BJIADOKC/samp-addon/master/build/client/d3d9.dll", ".\\d3d9.tmp", NULL, NULL);

	if(download == S_OK)
	{
		updaterLog("Downloaded update package (%i bytes)", boost::filesystem::file_size(tmpfile));
	}
	else
	{
		updaterLog("Error while downloading update package: %i (Error code: %i)", download, GetLastError());

		return 1;
	}

	try
	{
		if(boost::filesystem::exists(dllfile))
		{
			if(crc32_file(".\\d3d9.dll") == crc32_file(".\\d3d9.tmp"))
			{
				boost::filesystem::remove(tmpfile);

				updaterLog("Latest version of SAMP-Addon already installed, terminating...");

				return 1;
			}

			boost::filesystem::remove(dllfile);
		}

		boost::filesystem::rename(tmpfile, dllfile);
	}
	catch(boost::filesystem::filesystem_error &err)
	{
		updaterLog("Error while placing addon files (What: %s)", err.what());

		return 1;
	}

	updaterLog("Downloading changelog file...");

	try
	{
		if(boost::filesystem::exists(changelog))
			boost::filesystem::remove(changelog);
	}
	catch(boost::filesystem::filesystem_error &err)
	{
		updaterLog("Error while removing file 'addon_changelog.txt' (What: %s)", err.what());

		return 1;
	}

	download = URLDownloadToFile(NULL, "https://raw.githubusercontent.com/BJIADOKC/samp-addon/master/build/client/changelog.txt", ".\\addon_changelog.txt", NULL, NULL);

	if(download == S_OK)
	{
		updaterLog("Changes file saved to addon_changelog.txt");
	}
	else
	{
		updaterLog("Cannot download changelog file: %i (Error code: %i)", download, GetLastError());

		return 1;
	}

	updaterLog("Installation completed!");

	return 0;
}