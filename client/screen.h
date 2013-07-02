#pragma once



#include "addon.h"





class addonScreen
{

public:

	addonScreen();
	~addonScreen();

	void Get(std::string filename);

private:

	DWORD Address;
};