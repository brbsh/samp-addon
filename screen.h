#pragma once

#include "addon.h"





class addonScreen
{
public:
	bool screen;
	std::string filename;
	HANDLE screenHandle;

	addonScreen();
	~addonScreen();

	void Get(std::string filename);
	int Process();
};