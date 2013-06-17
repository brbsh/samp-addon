#pragma once



#include "fs.h"





addonFS *gFS;





void addonFS::RemoveFile(std::string file)
{
	remove(file.c_str());
}



std::vector<std::string> addonFS::ListDirectory(std::string folder)
{
	std::vector<std::string> ret;

	this->directory = opendir(folder.c_str());

	if(this->directory != NULL)
	{
		while(this->entry = readdir(this->directory))
			ret.push_back(this->entry->d_name);

		closedir(this->directory);

		return ret;
	}

	return ret;
}