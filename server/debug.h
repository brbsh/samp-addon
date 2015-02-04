#pragma once



#ifndef DEBUG_H
#define DEBUG_H

#include "server.h"





class amxDebug
{

public:

	amxDebug();
	virtual ~amxDebug();

	void processFW();
	void Log(char *format, ...);

	boost::thread *getThreadInstance() const
	{
		return threadInstance.get();
	}

	static void RemoteLog(char *format, ...);
	static void Thread();

private:

	std::queue<std::string> logQueue;
	boost::mutex lwMutex;
	boost::shared_ptr<boost::thread> threadInstance;
};





#endif