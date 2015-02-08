#pragma once



#include "client.hpp"





class addonCore
{

public:

	addonCore();
	virtual ~addonCore();

	void processFunc();
	void queueIN(boost::shared_ptr<boost::asio::ip::tcp::socket> sock);
	void queueOUT(boost::shared_ptr<boost::asio::ip::tcp::socket> sock);

	boost::thread *getThreadInstance() const
	{
		return threadInstance.get();
	}

	static void Thread();

private:

	boost::mutex pqMutex;
	boost::mutex ouMutex;

	std::queue<std::pair<UINT, std::string>> outputQueue;
	std::queue<std::pair<UINT, std::string>> pendingQueue;

	boost::shared_ptr<boost::thread> threadInstance;
};