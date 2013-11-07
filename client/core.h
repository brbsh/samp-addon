#pragma once



#include "client.h"





class addonCore
{

public:

	std::queue<std::pair<UINT, std::string>> outputQueue;
	std::queue<std::pair<UINT, std::string>> pendingQueue;

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