#pragma once



#include "server.hpp"





class amxUpdater
{

public:

	amxUpdater();
	virtual ~amxUpdater();

	boost::thread *getThreadInstance() const
	{
		return threadInstance.get();
	}

	static void updateThread(boost::asio::io_service& io_s, int local_plugin_crc);

private:

	boost::asio::io_service io_s;
	boost::shared_ptr<boost::thread> threadInstance;

	int local_plugin_crc;
};