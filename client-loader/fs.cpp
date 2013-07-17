#pragma once



#include "fs.h"





addonLoaderFS *gFS;





addonLoaderFS::addonLoaderFS()
{
	addonDebug("Loader FS constructor called");
}



addonLoaderFS::~addonLoaderFS()
{
	addonDebug("Loader FS deconstructor called");
}



void addonLoaderFS::RenameFile(std::string oldname, std::string newname)
{
	rename(oldname.c_str(), newname.c_str());
}



void addonLoaderFS::RemoveFile(std::string file)
{
	remove(file.c_str());
}



bool addonLoaderFS::FileExist(std::string file)
{
	std::fstream io;
	bool result = false;

	io.open(file.c_str(), std::fstream::in);
	result = io.good();
	io.close();

	return result;
}



std::vector<std::string> addonLoaderFS::ListDirectory(std::string folder)
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