#pragma once



#include "client.h"





class addonDebug
{

public:

	std::queue<std::string> Queue;
	std::vector<std::string> funcTrace;

	addonDebug();
	virtual ~addonDebug();

	void Log(char *format, ...);
	void traceLastFunction(char *format, ...);

	boost::mutex *getMutexInstance() const
	{
		return mutexInstance.get();
	}

	boost::thread *getThreadInstance() const
	{
		return threadInstance.get();
	}

	static void Thread();
	static LONG WINAPI UnhandledExceptionFilter(struct _EXCEPTION_POINTERS *ExceptionInfo);

private:

	boost::shared_ptr<boost::mutex> mutexInstance;
	boost::shared_ptr<boost::thread> threadInstance;
};