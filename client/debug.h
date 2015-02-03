#pragma once



#include "client.h"





class addonDebug
{

public:

	std::queue<std::string> logQueue;
	boost::mutex lwrMutex;
	std::list<std::string> funcTrace;

	addonDebug();
	virtual ~addonDebug();

	void Log(char *format, ...);
	void traceLastFunction(char *format, ...);

	boost::thread *getThreadInstance() const
	{
		return threadInstance.get();
	}

	static void Thread();
	static LONG WINAPI UnhandledExceptionFilter(struct _EXCEPTION_POINTERS *ExceptionInfo);

private:

	boost::mutex ftrMutex;
	boost::shared_ptr<boost::thread> threadInstance;
};