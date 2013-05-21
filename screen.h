#pragma once

#include "addon.h"





class addonScreen
{
public:
	bool screen;
	HANDLE screenHandle;

	addonScreen();
	~addonScreen();

	void Get();
	int Process(LPCTSTR szDest);
};