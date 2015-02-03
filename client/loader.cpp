#pragma once



#include "loader.h"





boost::shared_ptr<addonLoader> gLoader;


extern boost::shared_ptr<addonDebug> gDebug;
extern boost::shared_ptr<addonPool> gPool;





addonLoader::addonLoader()
{
	gDebug->traceLastFunction("addonLoader::addonLoader() at 0x?????");
	gDebug->Log("Called loader constructor");

	boost::mutex *tmpMutex = new boost::mutex();
	tmpMutex->initialize();

	boost::system::error_code error;

	boost::filesystem::path current(".");
	boost::filesystem::path addondll(".\\d3d9.dll");
	boost::filesystem::path updater(".\\addon_updater.tmp");
	boost::filesystem::path vorbishooked(".\\VorbisHooked.dll");
	boost::filesystem::path vorbisfile(".\\VorbisFile.dll");
	boost::filesystem::path gtasa(".\\gta_sa.exe");
	boost::filesystem::path gtasa_steam(".\\gta-sa.exe");

	boost::filesystem::directory_iterator end;

	if(!boost::filesystem::exists(gtasa) && !boost::filesystem::exists(gtasa_steam))
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

	if(!boost::filesystem::exists(updater))
	{
		HRESULT download = URLDownloadToFile(NULL, "https://raw.githubusercontent.com/BJIADOKC/samp-addon/master/build/client/updater.exe", ".\\addon_updater.tmp", NULL, NULL);

		if(download == S_OK)
		{
			STARTUPINFO updaterStart;
			PROCESS_INFORMATION updaterStartInfo;
			std::string cmdline_flags;

			ZeroMemory(&updaterStart, sizeof(updaterStart));
			updaterStart.cb = sizeof(updaterStart);

			ZeroMemory(&updaterStartInfo, sizeof(updaterStartInfo));

			cmdline_flags += "/checkforupdates";

			if(boost::filesystem::exists(vorbishooked))
				cmdline_flags += " /removeasiloader";

			if(CreateProcess("addon_updater.tmp", (LPSTR)cmdline_flags.c_str(), NULL, NULL, FALSE, CREATE_NEW_CONSOLE, NULL, NULL, &updaterStart, &updaterStartInfo))
			{
				gDebug->Log("Started updater daemon with flags: %s", cmdline_flags.c_str());
			}
			else
			{
				gDebug->Log("Cannot create process addon_updater.tmp: %i", GetLastError());

				MessageBox(NULL, "Error while creating process addon_updater.tmp", "SAMP-Addon", NULL);
			}
		}
	}

	/*if(boost::filesystem::exists(updater))
	{
		boost::filesystem::remove(updater, error);

		if(error)
		{
			MessageBox(NULL, "Cannot remove temp updater file", "SAMP-Addon", NULL);

			exit(EXIT_FAILURE);
		}
	}*/

	char sysdrive[5];

	DWORD serial = NULL;
	DWORD flags = NULL;

	std::string cmdline(GetCommandLine());

	//MessageBox(NULL, cmdline.c_str(), "Command Line", NULL);

	std::size_t name_ptr = (cmdline.find("-n") + 3);
	std::size_t ip_ptr = (cmdline.find("-h") + 3);
	std::size_t port_ptr = (cmdline.find("-p") + 3);
	std::size_t pass_ptr = (cmdline.find("-z") + 3);

	gPool->setVar("playerName", cmdline.substr(name_ptr, (ip_ptr - name_ptr - 4)), tmpMutex);
	gPool->setVar("serverIP", cmdline.substr(ip_ptr, (port_ptr - ip_ptr - 4)), tmpMutex);
	gPool->setVar("serverPort", cmdline.substr(port_ptr, 5), tmpMutex);

	if(pass_ptr != std::string::npos)
	{
		gPool->setVar("serverPassword", cmdline.substr(pass_ptr, INFINITE), tmpMutex);

		//password
	}

	strcpy_s(sysdrive, getenv("SystemDrive"));
	strcat_s(sysdrive, "\\");

	GetVolumeInformation(sysdrive, NULL, NULL, &serial, NULL, &flags, NULL, NULL);
	gPool->setVar("playerSerial", strFormat() << std::hex << std::uppercase << std::setw(8) << std::setfill('0') << serial << std::hex << std::uppercase << std::setw(8) << std::setfill('0') << flags, tmpMutex);

	boost::unordered_map<std::string, std::size_t> legal;

	HRESULT download = URLDownloadToFile(NULL, "https://raw.githubusercontent.com/BJIADOKC/samp-addon/master/build/client/whitelist.txt", ".\\addon_whitelist.tmp", NULL, NULL);

	if(download == S_OK)
	{
		std::ifstream f;
		std::string tmp;
		std::string fname;

		f.open("addon_whitelist.tmp", std::ifstream::in);

		while(std::getline(f, tmp))
		{
			fname = tmp.substr(0, tmp.find(" "));
			legal[fname] = atoi(tmp.substr((tmp.find(" ") + 1), INFINITE).c_str());
		}

		f.close();

		boost::filesystem::remove(boost::filesystem::path(".\\addon_whitelist.tmp"));
	}
	else
	{
		gDebug->Log("Error while retrieving whitelist file: %i (Error code: %i)", download, GetLastError());
	}
	
	gDebug->Log("\n\nLibrary whitelist: \n");

	for(boost::unordered_map<std::string, std::size_t>::iterator i = legal.begin(); i != legal.end(); i++)
	{
		gDebug->Log("%s (File size: %i)", i->first.c_str(), i->second);
	}

	bool isGood = false;

	for(boost::filesystem::directory_iterator file(current); file != end; file++)
	{
		if(boost::filesystem::is_regular_file(file->status()))
		{
			if(file->path() == addondll)
				continue;

			isGood = false;

			for(boost::unordered_map<std::string, std::size_t>::iterator i = legal.begin(); i != legal.end(); i++)
			{
				if((file->path().filename().string() == i->first) && (boost::filesystem::file_size(file->path()) == i->second))
				{
					isGood = true;

					break;
				}
			}

			if(!isGood)
			{
				// bad file
			}
			else
			{
				if(file->path().extension().string() == ".asi")
				{
					if(LoadLibrary(file->path().filename().string().c_str()))
						gDebug->Log("Plugin %s got loaded", file->path().filename().string().c_str());
					else
						gDebug->Log("Failed to load plugin %s", file->path().filename().string().c_str());
				}
			}
		}
		else if(boost::filesystem::is_directory(file->status()))
		{
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

	tmpMutex->destroy();
}



addonLoader::~addonLoader()
{
	gDebug->traceLastFunction("addonLoader::~addonLoader() at 0x?????");
	gDebug->Log("Called loader destructor");
}