#pragma once

#include "addon.h"




class addonSysexec
{
public:
	HANDLE sysexecHandle;

	addonSysexec();
	~addonSysexec();

	void Exec(std::string command);
};