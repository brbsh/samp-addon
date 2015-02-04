#pragma once



#ifndef CORE_H
#define CORE_H

#include "server.h"





class amxCore
{

public:

	std::list<AMX *> amxList;

	amxCore();
	virtual ~amxCore();

	void processFunc();

	boost::thread *getThreadInstance() const
	{
		return threadInstance.get();
	}

	static void Thread();

private:

	boost::shared_ptr<boost::thread> threadInstance;
};





#endif