#pragma once

#include "addon.h"
#include <dirent.h>





class addonFS
{

public:

	DIR *directory;
	struct dirent *entry;

	std::vector<std::string> Dir(std::string folder);
};