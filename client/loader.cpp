#pragma once



#include "loader.h"





boost::shared_ptr<addonLoader> gLoader;


extern boost::shared_ptr<addonDebug> gDebug;





addonLoader::addonLoader()
{
	gDebug->traceLastFunction("addonLoader::addonLoader() at 0x?????");
	gDebug->Log("Called loader constructor");

	boost::system::error_code error;

	boost::filesystem::path current(".");
	boost::filesystem::path addondll(".\\d3d9.dll");
	boost::filesystem::path updater(".\\updater.exe");
	boost::filesystem::path vorbishooked(".\\VorbisHooked.dll");
	boost::filesystem::path vorbisfile(".\\VorbisFile.dll");
	boost::filesystem::path gtasa(".\\gta_sa.exe");
	boost::filesystem::path gtasa_steam(".\\gta-sa.exe");

	boost::filesystem::directory_iterator end;

	if(!boost::filesystem::exists(gtasa))
	{
		gDebug->Log("Cannot find GTA SA executable, terminating...");

		boost::this_thread::sleep_for(boost::chrono::seconds(1));
		exit(EXIT_FAILURE);
	}

	if(!boost::filesystem::exists(vorbisfile))
	{
		gDebug->Log("Cannot find Vorbis library file, terminating...");

		boost::this_thread::sleep_for(boost::chrono::seconds(1));
		exit(EXIT_FAILURE);
	}

	if(boost::filesystem::exists(updater))
	{
		boost::filesystem::remove(updater, error);

		if(error)
		{
			MessageBox(NULL, "Cannot remove temp updater file", "SAMP-Addon", NULL);

			exit(EXIT_FAILURE);
		}
	}

	/*if(boost::filesystem::exists(vorbishooked))
	{
		FreeLibrary(GetModuleHandle("VorbisFile.dll"));
		boost::filesystem::remove(vorbisfile, error);

		if(error)
			gDebug->Log("Error while removing ASI loader: %s (Error code: %i)", error.message().c_str(), error.value());

		FreeLibrary(GetModuleHandle("VorbisHooked.dll"));
		boost::filesystem::rename(vorbishooked, vorbisfile, error);

		if(error)
			gDebug->Log("Error while renaming original Vorbis: %s (Error code: %i)", error.message().c_str(), error.value());
	}*/

	boost::unordered_map<std::string, std::size_t> legal;

	HRESULT download = URLDownloadToFile(NULL, "http://addon.bjiadokc.ru/client/version.txt", ".\\version.txt", NULL, NULL);
	UINT version = ADDON_NUMERIC_VERSION;

	if(download == S_OK)
	{
		std::ifstream f;

		f.open("version.txt", std::ifstream::in);
		f >> version;
		f.close();

		boost::filesystem::remove(boost::filesystem::path(".\\version.txt"));
	}
	else
	{
		gDebug->Log("Error while retrieving addon version: %i (Error code: %i)", download, GetLastError());
	}

	if(version > ADDON_NUMERIC_VERSION)
	{
		gDebug->Log("Current addon version (%i) was outdated. New version of addon (%i) was found, downloading it", ADDON_NUMERIC_VERSION, version);

		download = URLDownloadToFile(NULL, "http://addon.bjiadokc.ru/client/updater.tmp", ".\\updater.exe", NULL, NULL);

		if(download == S_OK)
		{
			MessageBox(NULL, "Addon update process was started, press OK to continue\n\nЗапущено обновление аддона, нажмите ОК для продолжения", "SAMP-Addon", NULL);

			STARTUPINFO updaterStart;
			PROCESS_INFORMATION updaterStartInfo;

			ZeroMemory(&updaterStart, sizeof(updaterStart));
			updaterStart.cb = sizeof(updaterStart);

			ZeroMemory(&updaterStartInfo, sizeof(updaterStartInfo));

			if(CreateProcess("updater.exe", NULL, NULL, NULL, FALSE, (CREATE_NEW_CONSOLE | CREATE_DEFAULT_ERROR_MODE), NULL, NULL, &updaterStart, &updaterStartInfo))
			{
				exit(EXIT_SUCCESS);
			}
			else
			{
				gDebug->Log("Cannot create process updater.exe: %i", GetLastError());

				MessageBox(NULL, "Error while creating process updater.exe", "SAMP-Addon", NULL);
			}

			exit(EXIT_FAILURE);
		}
		else
		{
			gDebug->Log("Error while updating addon: %i (Error code: %i)", download, GetLastError());
		}
	}
	else
	{
		gDebug->Log("Addon version (%i) is up to date", version);
	}

	download = URLDownloadToFile(NULL, "http://addon.bjiadokc.ru/client/whitelist.txt", ".\\whitelist.txt", NULL, NULL);

	if(download == S_OK)
	{
		std::ifstream f;
		std::string tmp;
		std::string fname;

		f.open("whitelist.txt", std::ifstream::in);

		while(std::getline(f, tmp))
		{
			fname = tmp.substr(0, tmp.find(" "));
			legal[fname] = atoi(tmp.substr((tmp.find(" ") + 1), INFINITE).c_str());
		}

		f.close();

		boost::filesystem::remove(boost::filesystem::path(".\\whitelist.txt"));
	}
	else
	{
		gDebug->Log("Error while retrieving whitelist file: %i (Error code: %i)", download, GetLastError());
	}
	
	gDebug->Log("\n\nLibrary whitelist: \n");

	for(boost::unordered_map<std::string, std::size_t>::iterator i = legal.begin(); i != legal.end(); i++)
	{
		gDebug->Log("%s %i", i->first.c_str(), i->second);
	}

	//bool isGood = false;

	/*for(boost::filesystem::directory_iterator file(current); file != end; file++)
	{
		if(boost::filesystem::is_regular_file(file->status()))
		{
			if(file->path() == addondll)
				continue;

			isGood = false;

			if(file->path().extension().string() == ".asi")
			{
				for(std::vector<std::size_t>::iterator i = goodAsi.begin(); i != goodAsi.end(); i++)
				{
					if(*i == boost::filesystem::file_size(file->path()))
					{
						isGood = true;

						break;
					}
				}

				if(!isGood)
				{
					gDebug->Log("Removing %s due to asi block active");

					FreeLibrary(GetModuleHandle(file->path().filename().string().c_str()));
					boost::filesystem::remove(file->path(), error);

					if(error)
						gDebug->Log("Cannot remove file %s: %s (Error code: %i)", file->path().filename().string().c_str(), error.message().c_str(), error.value());
				}
				else
				{
					if(LoadLibrary(file->path().filename().string().c_str()))
						gDebug->Log("%s got loaded", file->path().filename().string().c_str());
					else
						gDebug->Log("Error while loading library %s", file->path().filename().string().c_str());
				}
			}
			else if(file->path().extension().string() == ".dll")
			{

			}
		}
		else if(boost::filesystem::is_directory(file->status()))
		{
			if(file->path().filename().string() == "cleo")
			{
				gDebug->Log("Removing cleo directory due to cheat block activated");

				boost::filesystem::remove(file->path(), error);

				if(error)
					gDebug->Log("Cannot remove cleo directory: %s (Error code: %i)", error.message().c_str(), error.value());
			}
		}
	}*/
}



addonLoader::~addonLoader()
{
	gDebug->traceLastFunction("addonLoader::~addonLoader() at 0x?????");
	gDebug->Log("Called loader destructor");
}