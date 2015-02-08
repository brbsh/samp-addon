#pragma once



#include "server.hpp"





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