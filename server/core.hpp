#pragma once



#include "pool.hpp"
#include "server.hpp"





class amxCore
{

public:

	struct amxPush
	{
		amxPush()
		{

		}

		~amxPush()
		{

		}

		unsigned int clientid;

		std::vector<amxPool::svrData> args;
	};

	std::list<AMX *> amxList;

	amxCore();
	virtual ~amxCore();

	void processFunc(unsigned int maxclients);

	void pushToPT(unsigned short callback_id, amxPush data)
	{
		boost::unique_lock<boost::mutex> lockit(qMutex);
		amxQueue.push(std::make_pair(callback_id, data));
	}

	std::pair<unsigned short, amxPush> getFromPT()
	{
		boost::unique_lock<boost::mutex> lockit(qMutex);
		std::pair<unsigned short, amxPush> res = amxQueue.front();
		amxQueue.pop();

		return res;
	}

	bool isPTEmpty() const
	{
		return amxQueue.empty();
	}

	boost::thread *getThreadInstance() const
	{
		return threadInstance.get();
	}

	static void Thread();

private:

	boost::shared_ptr<boost::thread> threadInstance;
	std::queue< std::pair<unsigned short, amxPush> > amxQueue;
	boost::mutex qMutex;
};