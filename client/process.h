#pragma once



#include "addon.h"





class addonProcess
{

public:

	static void Thread();

	addonProcess();
	~addonProcess();

	bool threadActive;

	boost::mutex Mutex;
};