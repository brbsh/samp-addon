#pragma once



#include "updater.h"





addonUpdater *gUpdater;


extern addonHTTP *gHTTP;





addonUpdater::addonUpdater(std::size_t addon_loader_hash, std::size_t addon_dll_hash)
{
	boost::thread update(boost::bind(&addonUpdater::Thread, addon_loader_hash, addon_dll_hash));
}



addonUpdater::~addonUpdater()
{

}



void addonUpdater::Thread(std::size_t addon_loader_hash, std::size_t addon_dll_hash)
{
	addonDebug("Thread addonUpdater::Thread(%i, %i) successfuly started", addon_loader_hash, addon_dll_hash);

	std::string response;
	std::size_t hash;

	response = gHTTP->HTTP("bjiadokc.ru", "/addon/client/loader_hash.txt");
	hash = atoi(response.c_str());

	addonDebug("Last loader hash: %i", hash);

	if(hash != addon_loader_hash)
	{
		response = gHTTP->HTTP("bjiadokc.ru", "/addon/client/addon.asi");

		std::fstream file;

		file.open("SAMP\\addon\\update\\addon.asi", (std::fstream::out | std::fstream::binary));
		file << response;
		file.close();
	}

	response = gHTTP->HTTP("bjiadokc.ru", "/addon/client/addon_hash.txt");
	hash = atoi(response.c_str());

	addonDebug("Last addon hash: %i", hash);

	if(hash != addon_dll_hash)
	{
		response = gHTTP->HTTP("bjiadokc.ru", "/addon/client/addon.dll");

		std::fstream file;

		file.open("SAMP\\addon\\update\\addon.dll", (std::fstream::out | std::fstream::binary));
		file << response;
		file.close();
	}
}