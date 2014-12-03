#pragma once



#include "server.h"





class amxCore
{

public:

	std::list<AMX *> amxList;

	amxCore();
	virtual ~amxCore();

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