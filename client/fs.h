#pragma once



#include "addon.h"

#if defined WIN32
	#include "includes/dirent/dirent-win32.h"
#else
	#include "includes/dirent/dirent-linux.h"
#endif





class addonFS
{

public:

	addonFS();
	~addonFS();

	void RenameFile(std::string oldname, std::string newname);
	void RemoveFile(std::string file);
	bool FileExist(std::string file);

	std::vector<std::string> ListDirectory(std::string folder);

private:

	struct dirent *entry;
	DIR *directory;
};