#pragma once



#include "server.h"





class amxDebug
{

public:

	std::queue<std::string> logQueue;

	amxDebug();
	virtual ~amxDebug();

	void Log(char *format, ...);

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