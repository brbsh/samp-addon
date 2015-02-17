#pragma once



#include "server.hpp"





class amxCore
{

public:

	struct amxPush
	{
		std::string callback;
		std::string pushDataFirst;
		std::string pushDataSecond;
	};

	std::list<AMX *> amxList;

	amxCore();
	virtual ~amxCore();

	void processFunc(unsigned int maxclients);

	boost::thread *getThreadInstance() const
	{
		return threadInstance.get();
	}

	static void Thread();

private:

	boost::shared_ptr<boost::thread> threadInstance;
	std::queue<std::pair<unsigned int, amxPush> > amxQueue;
	boost::mutex qMutex;
};