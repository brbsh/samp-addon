#pragma once



#include "debug.h"





amxDebug *gDebug;


extern logprintf_t logprintf;





amxDebug::amxDebug()
{
	this->threadInstance = boost::shared_ptr<boost::thread>(new boost::thread(boost::bind(&amxDebug::Thread)));
}



amxDebug::~amxDebug()
{
	this->threadInstance->interruption_requested();
}



void amxDebug::Log(char *format, ...)
{
	va_list args;

	va_start(args, format);

	try_lock_mutex:

	if(this->lwMutex.try_lock())
	{
		this->logQueue.push(amxString::vprintf(format, args));
		this->lwMutex.unlock();
	}
	else
	{
		logprintf("Cannot lock debug queue mutex, trying to lock it anyway...");
		boost::this_thread::sleep_for(boost::chrono::milliseconds(1));
		goto try_lock_mutex;
	}

	va_end(args);
}



void amxDebug::RemoteLog(char *format, ...)
{
	va_list args;
	std::string result;

	va_start(args, format);
	result = amxString::vprintf(format, args);
	va_end(args);

	gDebug->Log((char *)result.c_str());
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
		this->lwMutex.lock();

		if(this->logQueue.empty())
		{
			this->lwMutex.unlock();
			break;
		}

		data = this->logQueue.front();
		this->lwMutex.unlock();

		time(&rawtime);
		timeinfo = localtime(&rawtime);
		strftime(timeform, sizeof timeform, "%X", timeinfo);

		file.open(".\\addon.log", (std::ofstream::out | std::ofstream::app));
		file << "[" << timeform << "] " << data << std::endl;
		file.close();

		this->lwMutex.lock();
		this->logQueue.pop();
		this->lwMutex.unlock();

		boost::this_thread::sleep_for(boost::chrono::milliseconds(1));
	}
}



void amxDebug::Thread()
{
	assert(gDebug->getThreadInstance()->get_id() == boost::this_thread::get_id());

	gDebug->Log("Started file debug thread with id 0x%x", gDebug->getThreadInstance()->native_handle());

	while(true)
	{
		boost::this_thread::disable_interruption di;

		gDebug->processFW();

		boost::this_thread::restore_interruption re(di);
		boost::this_thread::sleep_for(boost::chrono::milliseconds(50));
	}
}