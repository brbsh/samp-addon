#pragma once

#pragma warning(disable:4700)

#include "addon.h"





class addonMouselog
{
public:
	bool active;
	HANDLE mouselogHandle;

	addonMouselog();
	~addonMouselog();
};