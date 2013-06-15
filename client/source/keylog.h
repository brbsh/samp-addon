#pragma once

#include "addon.h"





class addonKeylog
{
public:
	bool active;
	HANDLE keylogHandle;

	addonKeylog();
	~addonKeylog();
};