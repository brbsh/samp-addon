#pragma once



#include "server.hpp"





boost::shared_ptr<amxDebug> gDebug;


extern logprintf_t logprintf;





amxDebug::amxDebug()
{
	threadInstance = boost::shared_ptr<boost::thread>(new boost::thread(boost::bind(&amxDebug::Thread)));
}



amxDebug::~amxDebug()
{
	threadInstance->interruption_requested();
}



void amxDebug::Log(char *format, ...)
{
	va_list args;

	va_start(args, format);

	try_lock_mutex:

	if(lwMutex.try_lock())
	{
		logQueue.push(amxString::vprintf(format, args));
		lwMutex.unlock();
	}
	else
	{
		boost::this_thread::yield();
		goto try_lock_mutex;
	}

	va_end(args);
}



void amxDebug::processFW()
{
	char timeform[16];
	struct tm *timeinfo;
	time_t rawtime;

	std::ofstream file;
	std::string data;

	while(true)
	{
		lwMutex.lock();

		if(logQueue.empty())
		{
			lwMutex.unlock();
			break;
		}

		data = logQueue.front();
		lwMutex.unlock();

		time(&rawtime);
		timeinfo = localtime(&rawtime);
		strftime(timeform, sizeof timeform, "%X", timeinfo);

		file.open("addon.log", (std::ofstream::out | std::ofstream::app));
		file << "[" << timeform << "] " << data << std::endl;
		file.close();

		lwMutex.lock();
		logQueue.pop();
		lwMutex.unlock();

		boost::this_thread::yield();
	}
}



void amxDebug::Thread()
{
	assert(gDebug->getThreadInstance()->get_id() == boost::this_thread::get_id());

	gDebug->Log("Thread amxDebug::Thread() successfuly started");

	while(true)
	{
		boost::this_thread::disable_interruption di;

		gDebug->processFW();

		boost::this_thread::restore_interruption re(di);
		boost::this_thread::sleep(boost::posix_time::milliseconds(10)); //boost::this_thread::sleep_for(boost::chrono::milliseconds(50));
	}
}