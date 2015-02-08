#pragma once



#include "client.hpp"





class addonDebug
{

public:

	std::list<std::string> funcTrace;

	addonDebug();
	virtual ~addonDebug();

	void Log(char *format, ...);
	void traceLastFunction(char *format, ...);
	void printBackTrace(std::ofstream& ref);
	void processFW();

	boost::thread *getThreadInstance() const
	{
		return threadInstance.get();
	}

	static void Thread();
	static LONG WINAPI UnhandledExceptionFilter(struct _EXCEPTION_POINTERS *ExceptionInfo);

private:

	std::queue<std::string> logQueue;
	boost::mutex lwrMutex;
	boost::shared_mutex ftrMutex;
	boost::shared_ptr<boost::thread> threadInstance;
};