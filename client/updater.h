#pragma once



#include "addon.h"





class addonUpdater
{

public:

	static void Thread(std::size_t addon_loader_hash, std::size_t addon_dll_hash);

	addonUpdater(std::size_t addon_loader_hash, std::size_t addon_dll_hash);
	~addonUpdater();
};