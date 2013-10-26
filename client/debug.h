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
	void TraceLastFunction(std::string funcdata);

	std::string GetLastFunction();
	std::string GetCurrentFunction();

	boost::mutex *getMutexInstance();
	boost::thread *getThreadInstance();

	void SetLastFunction(std::string funcname);
	void SetCurrentFunction(std::string funcname);

	static void Thread();
	static LONG WINAPI UnhandledExceptionFilter(struct _EXCEPTION_POINTERS *ExceptionInfo);

private:

	boost::shared_ptr<boost::mutex> mutexInstance;
	boost::shared_ptr<boost::thread> threadInstance;
};