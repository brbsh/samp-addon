#pragma once



#include "addon.h"
#include "includes/dirent.h"





class addonFS
{

public:

	addonFS();
	~addonFS();

	void RenameFile(std::string oldname, std::string newname);
	void RemoveFile(std::string file);
	std::vector<std::string> ListDirectory(std::string folder);

private:

	struct dirent *entry;
	DIR *directory;
};