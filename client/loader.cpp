#pragma once



#include "loader.h"





boost::shared_ptr<addonLoader> gLoader;


extern boost::shared_ptr<addonDebug> gDebug;





addonLoader::addonLoader()
{
	gDebug->traceLastFunction("addonLoader::addonLoader() at 0x?????");

	boost::system::error_code error;

	boost::filesystem::path addondll(".\\d3d9.dll");
	boost::filesystem::path vorbishooked(".\\VorbisHooked.dll");
	boost::filesystem::path vorbisfile(".\\VorbisFile.dll");
	boost::filesystem::path gtasa(".\\gta_sa.exe");

	boost::filesystem::directory_iterator end;

	if(!boost::filesystem::exists(gtasa))
	{
		gDebug->Log("Cannot find GTA SA executable, terminating...");
		exit(NULL);
	}

	if(!boost::filesystem::exists(vorbisfile))
	{
		gDebug->Log("Cannot find Vorbis library file, terminating...");
		exit(NULL);
	}

	if(boost::filesystem::exists(vorbishooked))
	{
		boost::filesystem::remove(vorbisfile, error);

		if(error)
			gDebug->Log("Error while removing ASI loader: %s (%i)", error.message().c_str(), boost::lexical_cast<int>(error));

		boost::filesystem::rename(vorbishooked, vorbisfile, error);

		if(error)
			gDebug->Log("Error while renaming original Vorbis: %s (%i)", error.message().c_str(), boost::lexical_cast<int>(error));
	}

	std::vector<std::size_t> goodAsi;
	std::vector<std::size_t> goodDll;

	std::string asi_white;
	std::string dll_white;

	asi_white = addonHTTP::Get("addon.bjiadokc.ru", "/asi.txt");
	dll_white = addonHTTP::Get("addon.bjiadokc.ru", "/dll.txt");
	
	gDebug->Log("asi: %s, dll: %s", asi_white.c_str(), dll_white.c_str());

	bool isGood = false;
	/*char *tokenizer;

	tokenizer = strtok((char *)asi_white.c_str(), " ");

	while(tokenizer)
	{
		goodAsi.push_back(atoi(tokenizer));

		tokenizer = strtok(NULL, " ");
	}

	tokenizer = strtok((char *)dll_white.c_str(), " ");

	while(tokenizer)
	{
		goodDll.push_back(atoi(tokenizer));

		tokenizer = strtok(NULL, " ");
	}*/

	for(boost::filesystem::directory_iterator file(boost::filesystem::path(".")); file != end; file++)
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
						isGood = true;
				}

				if(!isGood)
				{
					gDebug->Log("Removing %s due to asi block active");

					FreeLibrary(GetModuleHandle(file->path().filename().string().c_str()));
					boost::filesystem::remove(file->path(), error);

					if(error)
						gDebug->Log("Cannot remove file %s: %s (%i)", file->path().filename().string().c_str(), error.message().c_str(), boost::lexical_cast<int>(error));
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
					gDebug->Log("Cannot remove cleo directory: %s (%i)", error.message().c_str(), boost::lexical_cast<int>(error));
			}
		}
	}
}



addonLoader::~addonLoader()
{
	gDebug->traceLastFunction("addonLoader::~addonLoader() at 0x?????");
}