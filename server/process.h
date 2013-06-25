#pragma once



#include "addon.h"





class amxProcess
{

public:

	boost::mutex Mutex;

	static void Thread(AMX *amx);
};