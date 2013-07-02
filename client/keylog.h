#pragma once



#include "addon.h"





class addonKeylog
{

public:

	boost::mutex Mutex;

	static void Thread();
	bool threadActive;

	addonKeylog();
	~addonKeylog();
};