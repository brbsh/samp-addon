#pragma once



#include "client.hpp"





boost::shared_ptr<addonLoader> gLoader;


extern boost::shared_ptr<addonDebug> gDebug;
extern boost::shared_ptr<addonPool> gPool;





addonLoader::addonLoader()
{
	gDebug->traceLastFunction("addonLoader::addonLoader() at 0x?????");
	gDebug->Log("Called loader constructor");

	HRESULT download = NULL;

	boost::filesystem::path current(".");
	boost::filesystem::path addondll(".\\d3d9.dll");
	boost::filesystem::path updater(".\\addon_updater.exe");
	boost::filesystem::path vorbishooked(".\\VorbisHooked.dll");
	boost::filesystem::path vorbisfile(".\\VorbisFile.dll");
	boost::filesystem::path gtasa(".\\gta_sa.exe");
	//boost::filesystem::path gtasa_steam(".\\gta-sa.exe");

	boost::filesystem::directory_iterator end;

	if(!boost::filesystem::exists(gtasa))// && !boost::filesystem::exists(gtasa_steam))
	{
		gDebug->Log("Cannot find GTA SA executable, terminating...");

		boost::this_thread::sleep(boost::posix_time::seconds(1));
		exit(EXIT_FAILURE);
	}

	if(!boost::filesystem::exists(vorbisfile))
	{
		gDebug->Log("Cannot find Vorbis library file, terminating...");

		boost::this_thread::sleep(boost::posix_time::seconds(1));
		exit(EXIT_FAILURE);
	}

	if(!boost::filesystem::exists(updater))
	{
		download = URLDownloadToFile(NULL, "https://raw.githubusercontent.com/BJIADOKC/samp-addon/master/build/client/updater.exe", ".\\addon_updater.exe", NULL, NULL);

		if(download == S_OK)
		{
			gDebug->Log("Downloaded latest addon updater package (%i bytes)", boost::filesystem::file_size(updater));
		}
		else
		{
			gDebug->Log("Error while downloading addon updater package: %i (Error code: %i)", download, GetLastError());
			
			boost::this_thread::sleep(boost::posix_time::seconds(1));
			exit(EXIT_FAILURE);
		}
	}
	else
	{
		download = URLDownloadToFile(NULL, "https://raw.githubusercontent.com/BJIADOKC/samp-addon/master/build/client/updater_version.txt", ".\\updater_version.tmp", NULL, NULL);
		std::size_t hashcheck = addonHash::crc32_file(".\\addon_updater.exe");
		std::size_t hashcheck_remote = hashcheck;

		if(download == S_OK)
		{
			std::ifstream f;

			f.open(".\\updater_version.tmp", std::ifstream::in);
			f >> hashcheck_remote;
			f.close();

			boost::filesystem::remove(boost::filesystem::path(".\\updater_version.tmp"));

			gDebug->Log("Downloaded updater version file, latest updater version is %i", hashcheck_remote);
		}
		else
		{
			gDebug->Log("Error while downloading updater version file: %i (Error code: %i)", download, GetLastError());

			boost::this_thread::sleep(boost::posix_time::seconds(1));
			exit(EXIT_FAILURE);
		}

		if(hashcheck != hashcheck_remote)
		{
			HRESULT download = URLDownloadToFile(NULL, "https://raw.githubusercontent.com/BJIADOKC/samp-addon/master/build/client/updater.exe", ".\\addon_updater.exe", NULL, NULL);

			if(download == S_OK)
			{
				gDebug->Log("Downloaded latest addon updater package (%i bytes)", boost::filesystem::file_size(updater));
			}
			else
			{
				gDebug->Log("Error while downloading addon updater package: %i (Error code: %i)", download, GetLastError());

				boost::this_thread::sleep(boost::posix_time::seconds(1));
				exit(EXIT_FAILURE);
			}
		}
		else
		{
			gDebug->Log("Addon updater package is up to date (%i)", hashcheck);
		}
	}

	/*STARTUPINFO updaterStart;
	PROCESS_INFORMATION updaterStartInfo;
	std::string cmdline_flags;

	ZeroMemory(&updaterStart, sizeof(updaterStart));
	updaterStart.cb = sizeof(updaterStart);
	
	ZeroMemory(&updaterStartInfo, sizeof(updaterStartInfo));

	cmdline_flags += "/checkforupdates";

	if(boost::filesystem::exists(vorbishooked))
		cmdline_flags += " /removeasiloader";

	if(CreateProcess("addon_updater.exe", (LPSTR)cmdline_flags.c_str(), NULL, NULL, FALSE, CREATE_NO_WINDOW, NULL, NULL, &updaterStart, &updaterStartInfo))
	{
		gDebug->Log("Started updater daemon with flags: %s", cmdline_flags.c_str());
	}
	else
	{
		gDebug->Log("Cannot create process addon_updater.tmp: %i", GetLastError());

		boost::this_thread::sleep(boost::posix_time::seconds(1));
		exit(EXIT_FAILURE);
	}*/

	char sysdrive[32];

	DWORD serial = NULL;
	DWORD flags = NULL;

	std::string cmdline(GetCommandLine());

	std::size_t name_ptr = (cmdline.find("-n") + 3);
	std::size_t ip_ptr = (cmdline.find("-h") + 3);
	std::size_t port_ptr = (cmdline.find("-p") + 3);
	std::size_t pass_ptr = (cmdline.find("-z") + 3);

	gPool->setVar("name", cmdline.substr(name_ptr, (ip_ptr - name_ptr - 4)));
	gPool->setVar("server_ip", cmdline.substr(ip_ptr, (port_ptr - ip_ptr - 4)));
	gPool->setVar("server_port", cmdline.substr(port_ptr, 5));

	if(pass_ptr != std::string::npos)
	{
		gPool->setVar("server_password", cmdline.substr(pass_ptr, INFINITE));

		//password
	}

	strcpy_s(sysdrive, getenv("SystemDrive"));
	strcat_s(sysdrive, "\\");

	GetVolumeInformation(sysdrive, NULL, NULL, &serial, NULL, &flags, NULL, NULL);
	gPool->setVar("serial", strFormat() << UNIQUE_HEX_MOD << serial << UNIQUE_HEX_MOD << (serial ^ flags));

	boost::unordered_map<std::string, std::size_t> legal;
	boost::unordered_map<std::string, HMODULE> loaded;

	download = URLDownloadToFile(NULL, "https://raw.githubusercontent.com/BJIADOKC/samp-addon/master/build/client/whitelist.txt", ".\\addon_whitelist.tmp", NULL, NULL);

	if(download == S_OK)
	{
		std::ifstream f;
		std::string tmp;
		std::string fname;

		f.open("addon_whitelist.tmp", std::ifstream::in);

		while(std::getline(f, tmp))
		{
			fname = tmp.substr(0, tmp.find(" "));
			legal[fname] = boost::lexical_cast<int>(tmp.substr((tmp.find(" ") + 1), INFINITE));
		}

		f.close();

		boost::filesystem::remove(boost::filesystem::path(".\\addon_whitelist.tmp"));
	}
	else
	{
		gDebug->Log("Error while retrieving whitelist file: %i (Error code: %i)", download, GetLastError());
	}
	
	gDebug->Log("\tLibrary whitelist:");

	for(boost::unordered_map<std::string, std::size_t>::iterator i = legal.begin(); i != legal.end(); i++)
	{
		gDebug->Log("   %s\t(CRC32: %i)", i->first.c_str(), i->second);
	}

	bool isGood;
	std::string filename;

	for(boost::filesystem::directory_iterator file(current); file != end; file++)
	{
		if(boost::filesystem::is_regular_file(file->status()))
		{
			if(file->path() == addondll)
				continue;

			isGood = false;

			for(boost::unordered_map<std::string, std::size_t>::iterator i = legal.begin(); i != legal.end(); i++)
			{
				filename = file->path().filename().string();

				if((filename == i->first) && (addonHash::crc32_file(filename) == i->second))
				{
					isGood = true;

					break;
				}
			}

			if(!isGood)
			{
				//gDebug->Log(" %s -> %i", filename.c_str(), crc32_file(filename));
			}
			else
			{
				if(file->path().extension().string() == ".asi")
				{
					HMODULE addr = NULL;

					if((addr = LoadLibrary(filename.c_str())))
					{
						gDebug->Log(" ASI plugin %s got loaded", filename.c_str());

						loaded[filename] = addr;
					}
					else
					{
						gDebug->Log(" Failed to load plugin %s (What: %i)", filename.c_str(), GetLastError());
					}
				}
			}
		}
		else if(boost::filesystem::is_directory(file->status()))
		{
			isGood = false;

			if(file->path().filename().string() == "cleo")
			{
				// cleo dir found
			}
			else if(file->path().filename().string() == "mod_sa")
			{
				// m0d_sa dir found
			}
		}
	}

	gDebug->Log("\tLoaded plugins:");

	for(boost::unordered_map<std::string, HMODULE>::iterator i = loaded.begin(); i != loaded.end(); i++)
	{
		gDebug->Log("   %s -> 0x%08x", (*i).first.c_str(), (*i).second);
	}
}



addonLoader::~addonLoader()
{
	gDebug->traceLastFunction("addonLoader::~addonLoader() at 0x?????");
	gDebug->Log("Called loader destructor");
}