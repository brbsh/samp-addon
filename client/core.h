#pragma once



#include "client.h"





class addonCore
{

public:

	addonCore();
	virtual ~addonCore();

	boost::mutex *getMutexInstance() const
	{
		return mutexInstance.get();
	}

	boost::thread *getThreadInstance() const
	{
		return threadInstance.get();
	}

	static void Thread();

private:

	boost::shared_ptr<boost::mutex> mutexInstance;
	boost::shared_ptr<boost::thread> threadInstance;
};