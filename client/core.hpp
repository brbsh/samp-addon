#pragma once



#include "client.hpp"





class addonCore
{

public:

	addonCore();
	virtual ~addonCore();

	void processFunc();
	void queueIN(addonSocket *instance);

	boost::thread *getThreadInstance() const
	{
		return threadInstance.get();
	}

	static void Thread();

private:

	boost::mutex pqMutex;
	std::queue<std::pair<unsigned int, std::string>> pendingQueue;

	boost::shared_ptr<boost::thread> threadInstance;
};