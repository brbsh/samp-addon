#pragma once



#include "addon.h"





class addonProcess
{

public:

	boost::mutex Mutex;

	static void Thread();
	bool threadActive;

	addonProcess();
	~addonProcess();
};