#pragma once



#include "fs.h"





addonFS *gFS;





addonFS::addonFS()
{
	addonDebug("FS constructor called\n");
}



addonFS::~addonFS()
{
	addonDebug("FS deconstructor called");
}



void addonFS::RenameFile(std::string oldname, std::string newname)
{
	rename(oldname.c_str(), newname.c_str());
}



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
		{
			if(!strcmp(this->entry->d_name, ".") || !strcmp(this->entry->d_name, ".."))
				continue;

			ret.push_back(this->entry->d_name);
		}

		closedir(this->directory);

		return ret;
	}

	return ret;
}