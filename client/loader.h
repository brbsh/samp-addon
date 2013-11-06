#pragma once



#include "client.h"





class addonLoader
{

public:

	addonLoader();
	virtual ~addonLoader();

	boost::mutex *getMutexInstance();
	boost::thread *getThreadInstance();

private:

	boost::shared_ptr<boost::mutex> mutexInstance;
	boost::shared_ptr<boost::thread> threadInstance;
};