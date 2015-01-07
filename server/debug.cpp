#pragma once



#include "debug.h"





boost::shared_ptr<amxDebug> gDebug;





amxDebug::amxDebug()
{
	this->mutexInstance = boost::shared_ptr<boost::mutex>(new boost::mutex());
	this->threadInstance = boost::shared_ptr<boost::thread>(new boost::thread(boost::bind(&amxDebug::Thread)));
}



amxDebug::~amxDebug()
{
	this->threadInstance->interruption_requested();
	//this->mutexInstance->destroy();
}



void amxDebug::Log(char *format, ...)
{
	va_list args;
	boost::mutex logMutex;

	va_start(args, format);

	logMutex.lock();
	this->logQueue.push(amxString::vprintf(format, args));
	logMutex.unlock();

	va_end(args);

	//logMutex.destroy();
}



void amxDebug::Thread()
{
	assert(gDebug->getThreadInstance()->get_id() == boost::this_thread::get_id());

	gDebug->Log("Started file debug thread with id 0x%x", gDebug->getThreadInstance()->native_handle());

	char timeform[16];
	struct tm *timeinfo;
	time_t rawtime;

	std::ofstream file;
	std::string data;

	while(true)
	{
		while(!gDebug->logQueue.empty())
		{
			boost::this_thread::disable_interruption di;

			gDebug->getMutexInstance()->lock();
			data = gDebug->logQueue.front();
			gDebug->logQueue.pop();
			gDebug->getMutexInstance()->unlock();

			time(&rawtime);
			timeinfo = localtime(&rawtime);
			strftime(timeform, sizeof timeform, "%X", timeinfo);

			file.open("addon.log", (std::ofstream::out | std::ofstream::app));
			file << "[" << timeform << "] " << data << std::endl;
			file.close();

			boost::this_thread::restore_interruption re(di);
			boost::this_thread::sleep_for(boost::chrono::milliseconds(10));
		}

		boost::this_thread::sleep_for(boost::chrono::milliseconds(50));
	}
}