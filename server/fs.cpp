#pragma once



#include "fs.h"





amxFS *gFS;





amxFS::amxFS()
{
	addonDebug("FS constructor called");
}



amxFS::~amxFS()
{
	addonDebug("FS deconstructor called");
}



void amxFS::RenameFile(std::string oldname, std::string newname)
{
	rename(oldname.c_str(), newname.c_str());
}



void amxFS::RemoveFile(std::string file)
{
	remove(file.c_str());
}



bool amxFS::FileExist(std::string file)
{
	bool result = false;

	std::fstream io;

	io.open(file.c_str(), std::fstream::in);
	result = io.good();
	io.close();

	return result;
}