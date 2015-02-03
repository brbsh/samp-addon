#pragma once



#include "client.h"





class addonCore
{

public:

	std::queue<std::pair<UINT, std::string>> outputQueue;
	std::queue<std::pair<UINT, std::string>> pendingQueue;
	boost::mutex pqMutex;

	addonCore();
	virtual ~addonCore();

	void Queue(std::pair<UINT, std::string> set);

	boost::thread *getThreadInstance() const
	{
		return threadInstance.get();
	}

	static void Thread();

private:

	boost::shared_ptr<boost::thread> threadInstance;
};