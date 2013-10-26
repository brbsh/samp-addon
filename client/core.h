#pragma once



#include "client.h"





class addonCore
{

public:

	addonCore();
	virtual ~addonCore();

	boost::mutex *getMutexInstance();
	boost::thread *getThreadInstance();

	static void Thread();

private:

	boost::shared_ptr<boost::mutex> mutexInstance;
	boost::shared_ptr<boost::thread> threadInstance;
};