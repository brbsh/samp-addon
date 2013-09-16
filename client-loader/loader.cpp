#pragma once



#include "loader.h"





BOOL WINAPI DllMain(HINSTANCE hinstDLL, DWORD fdwReason, LPVOID lpvReserved)
{
	if(fdwReason == DLL_PROCESS_ATTACH)
	{
		boost::system::error_code error;
		boost::filesystem::path gtadir(".");
		boost::filesystem::path addondir("SAMP\\addon");
		boost::filesystem::path updatedir("SAMP\\addon\\update");
		boost::filesystem::path addon_loader(".\\addon.asi");
		boost::filesystem::path addon_temp_loader(".\\_addon.asi");
		boost::filesystem::path addon_dll("SAMP\\addon\\addon.dll");
		boost::filesystem::path addon_log("SAMP\\addon\\addon.log");
		boost::filesystem::directory_iterator end;

		DisableThreadLibraryCalls(hinstDLL);

		if(GetModuleHandleW(L"_addon.asi") == /*boost::lexical_cast<HMODULE>(*/hinstDLL)//)
		{
			std::size_t old_file_size = boost::filesystem::file_size(addon_loader, error);

			if(error)
			{
				addonDebug("Cannot calculate size of file 'addon.asi' (Error code: %i)", boost::lexical_cast<int>(error));

				return false;
			}

			std::size_t new_file_size = boost::filesystem::file_size(addon_temp_loader, error);

			if(error)
			{
				addonDebug("Cannot calculate size of file '_addon.asi' (Error code: %i)", boost::lexical_cast<int>(error));

				return false;
			}

			boost::filesystem::copy(addon_temp_loader, addon_loader, error);

			if(error)
			{
				addonDebug("Cannot update addon.asi (Error code: %i)", boost::lexical_cast<int>(error));

				MessageBoxW(NULL, L"Cannot update addon loader, please post about this bug (including file addon.log) to https://github.com/BJIADOKC/samp-addon/issues", L"SAMP-Addon", NULL);
			}
			else
				addonDebug("addon.asi (%i) was updated to newest version (%i)", old_file_size, new_file_size);

			return false;
		}

		if(boost::filesystem::exists(addon_log))
		{
			boost::filesystem::remove(addon_log, error);

			if(error)
				addonDebug("Cannot remove old log file 'addon.log' (Error code: %i)", boost::lexical_cast<int>(error));
		}

		addonDebug("\tDebugging started\n");
		addonDebug("-----------------------------------------------------------------");
		addonDebug("Called asi attach | asi base address: 0x%x | attach reason: %i | reserved: %i", hinstDLL, fdwReason, lpvReserved);
		addonDebug("-----------------------------------------------------------------");

		if(boost::filesystem::exists(addon_temp_loader))
		{
			addonDebug("Removing old instance of addon loader '_addon.asi'");

			boost::filesystem::remove(addon_temp_loader, error);

			if(error)
			{
				addonDebug("Cannot remove old instance of addon loader '_addon.asi' (Error code: %i)", boost::lexical_cast<int>(error));

				return false;
			}

			addonDebug("Old instance of addon loader '_addon.asi' was removed");
		}

		if(!boost::filesystem::exists(addon_loader) || (GetModuleHandleW(L"addon.asi") != /*boost::lexical_cast<HMODULE>(*/hinstDLL))//)
		{
			addonDebug("Addon loader must be named as 'addon.asi'. Please, rename it");

			return false;
		}

		if(GetModuleHandleW(L"addon.dll"))
		{
			addonDebug("Addon library already injected, terminating...");

			return false;
		}

		for(boost::filesystem::directory_iterator dir(updatedir); dir != end; dir++)
		{
			if(boost::filesystem::is_regular_file(dir->status()))
			{
				if(dir->path().filename().string() == "addon.asi")
				{
					boost::filesystem::copy_file(dir->path(), addon_temp_loader, error);

					if(error)
					{
						addonDebug("Cannot copy SAMP\\addon\\addon.asi to \\ (Error code: %i)", boost::lexical_cast<int>(error));
						MessageBoxW(NULL, L"Cannot update addon loader, please post about this bug (including file addon.log) to https://github.com/BJIADOKC/samp-addon/issues", L"SAMP-Addon", NULL);

						return false;
					}
					else
						addonDebug("Updated addon.asi was copied to temp loader '_addon.asi'");

					boost::filesystem::remove(dir->path(), error);

					if(error)
					{
						addonDebug("Cannot remove SAMP\\addon\\addon.asi (Error code: %i)", boost::lexical_cast<int>(error));
						MessageBoxW(NULL, L"Cannot update addon loader, please post about this bug (including file addon.log) to https://github.com/BJIADOKC/samp-addon/issues", L"SAMP-Addon", NULL);

						return false;
					}
					else
						addonDebug("Update file addon.asi cleanup successful");

					MessageBoxW(NULL, L"Addon loader was updated, please, reopen game\n\nЗагрузчик аддона был обновлен, перезапустите игру", L"SAMP-Addon", NULL);
					exit(NULL);

					return true;
				}

				if(dir->path().filename().string() == "addon.dll")
				{
					boost::filesystem::copy(dir->path(), addon_dll, error);

					if(error)
					{
						addonDebug("Cannot copy SAMP\\addon\\addon.asi to \\ (Error code: %i)", boost::lexical_cast<int>(error));
						MessageBoxW(NULL, L"Cannot update addon loader, please post about this bug (including file addon.log) to https://github.com/BJIADOKC/samp-addon/issues", L"SAMP-Addon", NULL);

						return false;
					}
					else
						addonDebug("Updated addon.dll was copied to SAMP\\addon\\addon.dll");

					boost::filesystem::remove(dir->path(), error);

					if(error)
					{
						addonDebug("Cannot remove SAMP\\addon\\addon.dll (Error code: %i)", boost::lexical_cast<int>(error));
						MessageBoxW(NULL, L"Cannot update addon module, please post about this bug (including file addon.log) to https://github.com/BJIADOKC/samp-addon/issues", L"SAMP-Addon", NULL);

						return false;
					}
					else
						addonDebug("Update file addon.dll cleanup successful");

					continue;
				}

				addonDebug("Invalid file '%s' in addon update directory", dir->path().filename().string().c_str());
			}
		}

		bool good = false;

		std::string file_name;
		std::string file_ext;
		std::size_t file_size;
		std::vector<std::size_t> good_asi_sizes;

		good_asi_sizes.push_back(98816); // audio.asi (Audio plugin v0.5 R2)

		for(boost::filesystem::directory_iterator dir(gtadir); dir != end; dir++)
		{
			if(boost::filesystem::is_regular_file(dir->status()))
			{
				if(dir->path() == addon_loader)
					continue;

				file_name = dir->path().filename().string();
				file_ext = dir->path().extension().string();
				file_size = boost::filesystem::file_size(dir->path());

				if(file_name == "d3d9.dll")
				{
					addonDebug("d3d9.dll (%i) found in GTA folder, checking it", file_size);

					continue;
				}

				if(file_ext != ".asi")
					continue;

				good = false;

				addonDebug("asi %s size is %i", file_name.c_str(), file_size);

				for(std::vector<std::size_t>::iterator i = good_asi_sizes.begin(); i != good_asi_sizes.end(); i++)
				{
					if(file_size == *i)
					{
						addonDebug("File %s in found in whitelist", file_name.c_str());

						good = true;

						continue;
					}
				}

				if(good)
					continue;

				addonDebug("Renaming '%s' due to asi block activated", file_name.c_str());

				file_name.push_back('_');
				boost::filesystem::rename(dir->path(), boost::filesystem::path(file_name), error);

				if(error)
				{
					addonDebug("Security warning: cannot rename file %s (Error code: %i)", dir->path().filename().string().c_str(), boost::lexical_cast<int>(error));

					return false;
				}
				else
					addonDebug("File %s was renamed to %s", dir->path().filename().string().c_str(), file_name.c_str());
			}
			else if(boost::filesystem::is_directory(dir->status()))
			{
				file_name = dir->path().filename().string();

				if(file_name == "cleo") // cleo dir
				{
					addonDebug("Renaming directory '%s' due to cleo block activated", file_name.c_str());

					file_name.push_back('_');
					boost::filesystem::rename(dir->path(), boost::filesystem::path(file_name), error);

					if(error)
					{
						addonDebug("Security warning: cannot rename directory %s (Error code: %i)", dir->path().filename().string().c_str(), boost::lexical_cast<int>(error));

						return false;
					}
					else
						addonDebug("Directory %s was renamed to %s", dir->path().filename().string().c_str(), file_name.c_str());
				}
			}
		}

		if(!GetModuleHandleW(L"gta_sa.exe") || !GetModuleHandleW(L"Vorbis.dll") || !GetModuleHandleW(L"VorbisFile.dll") || !GetModuleHandleW(L"VorbisHooked.dll"))
		{
			addonDebug("Loader attached from unknown process, terminating");

			return false;
		}

		addonDebug("Game process base address: 0x%x", GetModuleHandleW(L"gta_sa.exe"));
		addonDebug("Vorbis library base address: 0x%x", GetModuleHandleW(L"Vorbis.dll"));
		addonDebug("VorbisFile hook base address: 0x%x", GetModuleHandleW(L"VorbisFile.dll"));
		addonDebug("VorbisFile library base address: 0x%x", GetModuleHandleW(L"VorbisHooked.dll"));
		addonDebug("Addon loader base address: 0x%x", hinstDLL);

		std::size_t addon_loader_hash = boost::filesystem::file_size(addon_loader);
		std::size_t addon_dll_hash = boost::filesystem::file_size(addon_dll);

		addonDebug("Addon loader size: %i", addon_loader_hash);
		addonDebug("Addon dll size: %i", addon_dll_hash);

		boost::thread load(boost::bind(&addonLoader::Thread, hinstDLL, addon_loader_hash, addon_dll_hash));
		//load.detach();
	}
	else if(fdwReason == DLL_PROCESS_DETACH)
	{
		std::string file_name;
		std::string file_ext;

		boost::system::error_code error;
		boost::filesystem::path gtadir(".");
		boost::filesystem::directory_iterator end;

		for(boost::filesystem::directory_iterator dir(gtadir); dir != end; dir++)
		{
			if(boost::filesystem::is_regular_file(dir->status()))
			{
				file_name = dir->path().filename().string();
				file_ext = dir->path().extension().string();

				if(file_ext != ".asi_")
					continue;

				addonDebug("Renaming '%s' due to asi block deactivated", file_name.c_str());

				file_name.pop_back();
				boost::filesystem::rename(dir->path(), boost::filesystem::path(file_name), error);

				if(error)
				{
					addonDebug("Security warning: cannot rename file %s (Error code: %i)", dir->path().filename().string().c_str(), boost::lexical_cast<int>(error));

					return false;
				}
				else
					addonDebug("File %s was renamed to %s", dir->path().filename().string().c_str(), file_name.c_str());
			}
			else if(boost::filesystem::is_directory(dir->status()))
			{
				file_name = dir->path().filename().string();

				if(file_name == "cleo_")
				{
					addonDebug("Renaming directory '%s' due to cleo block deactivated", file_name.c_str());

					file_name.pop_back();
					boost::filesystem::rename(dir->path(), boost::filesystem::path(file_name), error);

					if(error)
					{
						addonDebug("Security warning: cannot rename directory %s (Error code: %i)", dir->path().filename().string().c_str(), boost::lexical_cast<int>(error));

						return false;
					}
					else
						addonDebug("Directory %s was renamed to %s", dir->path().filename().string().c_str(), file_name.c_str());
				}
			}
		}

		addonDebug("-----------------------------------------------------------------");
		addonDebug("Called asi detach | asi base address: 0x%x | detach reason: %i | reserved: %i", hinstDLL, fdwReason, lpvReserved);
		addonDebug("-----------------------------------------------------------------\n");
		addonDebug("\tDebugging stopped");
	}

	return true;
}



void addonLoader::Thread(HINSTANCE addon_loader_address, std::size_t addon_loader_hash, std::size_t addon_dll_hash)
{
	DWORD oldProt;
	VirtualProtect((LPVOID)0x401000, 0x4A3000, PAGE_EXECUTE_READWRITE, &oldProt);

	while(*(int *)0xB6F5F0 == 0) // RenderWare load flag
	{
		boost::this_thread::sleep(boost::posix_time::milliseconds(100));
	}

	addonDebug("Thread addonLoader::Thread(0x%x, %i, %i) successfuly started", addon_loader_address, addon_loader_hash, addon_dll_hash);

	if(!GetModuleHandleW(L"samp.dll"))
	{
		addonDebug("Singleplayer loaded, terminating");

		return;
	}

	SetDllDirectoryW(L"SAMP\\addon");
	HMODULE addon = LoadLibraryW(L"addon.dll");

	if(!addon)
	{
		addonDebug("Cannot launch SAMP\\addon\\addon.dll (Error code: %i)", GetLastError());

		return;
	}

	addonLoad addon_start = (addonLoad)GetProcAddress(addon, "addon_start");

	if(!addon_start)
	{
		addonDebug("Cannot export main loader function (Error code: %i)", GetLastError());

		return;
	}

	addon_start(addon_loader_address, addon_loader_hash, addon, addon_dll_hash);
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
	char *buffer = new char[++length];

	vsnprintf(buffer, length, text, args);
	va_end(args);

	logfile.open("SAMP\\addon\\addon.log", (std::fstream::out | std::fstream::app));
	logfile << "[" << timeform << "] " << buffer << std::endl;
	logfile.close();

	delete[] buffer;
}