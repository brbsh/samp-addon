#pragma once



#include "debug.h"





boost::shared_ptr<addonDebug> gDebug;





addonDebug::addonDebug()
{
	SetUnhandledExceptionFilter(addonDebug::UnhandledExceptionFilter);

	boost::filesystem::path log(".\\addon_log.txt");

	if(boost::filesystem::exists(log))
	{
		boost::system::error_code error;

		boost::filesystem::remove(log, error);

		//if(error)
			//gDebug->Log("Cannot remove old log file: %s (Error code: %i)", error.message().c_str(), error.value());
	}

	this->threadInstance = boost::shared_ptr<boost::thread>(new boost::thread(boost::bind(&addonDebug::Thread)));

	this->traceLastFunction("addonDebug::addonDebug() at 0x?????");
	this->Log("Called debug constructor");
}



addonDebug::~addonDebug()
{
	this->traceLastFunction("addonDebug::~addonDebug() at 0x?????");
	this->Log("Called debug destructor");

	this->threadInstance->interruption_requested();
}



LONG WINAPI addonDebug::UnhandledExceptionFilter(struct _EXCEPTION_POINTERS *ExceptionInfo)
{
	static bool processingException = false;

	if(processingException)
		return EXCEPTION_CONTINUE_EXECUTION;

	processingException = true;

	/*boost::filesystem::path log(".\\addon_crashreport.txt");

	if(boost::filesystem::exists(log))
	{
		boost::system::error_code error;

		boost::filesystem::remove(log, error);

		if(error)
			gDebug->Log("Cannot remove old crashreport file: %s (Error code: %i)", error.message().c_str(), error.value());
	}*/

	std::ofstream report;

	report.open(".\\addon_crashreport.txt", std::ofstream::out);

	report << "-----------------------------------------------------------------" << std::endl;
	report << "\t\t     SAMP-Addon was crashed" << std::endl;
	report << "-----------------------------------------------------------------" << std::endl << std::endl;
	report << "GTA base address: 0x" << std::hex << std::uppercase << std::setw(8) << std::setfill('0') << GetModuleHandle(NULL) << ", SA:MP base address: 0x" << std::hex << std::uppercase << std::setw(8) << std::setfill('0') << GetModuleHandle("samp.dll") << std::endl;
	report << "Exception at address: 0x" << std::hex << std::uppercase << std::setw(8) << std::setfill('0') << ExceptionInfo->ExceptionRecord->ExceptionAddress << "  ||  ";

	switch(ExceptionInfo->ExceptionRecord->ExceptionCode)
	{
		case EXCEPTION_ACCESS_VIOLATION:
		{
			report << "What: (EXCEPTION_ACCESS_VIOLATION) ";

			if(ExceptionInfo->ExceptionRecord->ExceptionInformation[0] == 0)
			{
				// bad read
				report << "Attempted to read from: 0x" << std::hex << std::uppercase << std::setw(8) << std::setfill('0') << ExceptionInfo->ExceptionRecord->ExceptionInformation[1] << std::endl;
			}
			else if(ExceptionInfo->ExceptionRecord->ExceptionInformation[0] == 1)
			{
				// bad write
				report << "Attempted to write to: 0x" << std::hex << std::uppercase << std::setw(8) << std::setfill('0') << ExceptionInfo->ExceptionRecord->ExceptionInformation[1] << std::endl;
			}
			else if(ExceptionInfo->ExceptionRecord->ExceptionInformation[0] == 8)
			{
				// user-mode data execution prevention (DEP)
				report << "Data Execution Prevention (DEP) at: 0x" << std::hex << std::uppercase << std::setw(8) << std::setfill('0') << ExceptionInfo->ExceptionRecord->ExceptionInformation[1] << std::endl;
			}
			else
			{
				// unknown, shouldn't happen
				report << "Unknown access violation at: 0x" << std::hex << std::uppercase << std::setw(8) << std::setfill('0') << ExceptionInfo->ExceptionRecord->ExceptionInformation[1] << std::endl;
			}
		}
		break;

		case EXCEPTION_ARRAY_BOUNDS_EXCEEDED:
			report << "What: EXCEPTION_ARRAY_BOUNDS_EXCEEDED" << std::endl;
			break;

		case EXCEPTION_BREAKPOINT:
			report << "What: EXCEPTION_BREAKPOINT" << std::endl;
			break;

		case EXCEPTION_DATATYPE_MISALIGNMENT:
			report << "What: EXCEPTION_DATATYPE_MISALIGNMENT" << std::endl;
			break;

		case EXCEPTION_FLT_DENORMAL_OPERAND:
			report << "What: EXCEPTION_FLT_DENORMAL_OPERAND" << std::endl;
			break;

		case EXCEPTION_FLT_DIVIDE_BY_ZERO:
			report << "What: EXCEPTION_FLT_DIVIDE_BY_ZERO" << std::endl;
			break;

		case EXCEPTION_FLT_INEXACT_RESULT:
			report << "What: EXCEPTION_FLT_INEXACT_RESULT" << std::endl;
			break;

		case EXCEPTION_FLT_INVALID_OPERATION:
			report << "What: EXCEPTION_FLT_INVALID_OPERATION" << std::endl;
			break;

		case EXCEPTION_FLT_OVERFLOW:
			report << "What: EXCEPTION_FLT_OVERFLOW" << std::endl;
			break;

		case EXCEPTION_FLT_STACK_CHECK:
			report << "What: EXCEPTION_FLT_STACK_CHECK" << std::endl;
			break;

		case EXCEPTION_FLT_UNDERFLOW:
			report << "What: EXCEPTION_FLT_UNDERFLOW" << std::endl;
			break;

		case EXCEPTION_ILLEGAL_INSTRUCTION:
			report << "What: EXCEPTION_ILLEGAL_INSTRUCTION" << std::endl;
			break;

		case EXCEPTION_IN_PAGE_ERROR:
		{
			report << "What: (EXCEPTION_IN_PAGE_ERROR) ";

			if(ExceptionInfo->ExceptionRecord->ExceptionInformation[0] == 0)
			{
				// bad read
				report << "Attempted to read from: 0x" << std::hex << std::uppercase << std::setw(8) << std::setfill('0') << ExceptionInfo->ExceptionRecord->ExceptionInformation[1] << std::endl;
			}
			else if(ExceptionInfo->ExceptionRecord->ExceptionInformation[0] == 1)
			{
				// bad write
				report << "Attempted to write to: 0x" << std::hex << std::uppercase << std::setw(8) << std::setfill('0') << ExceptionInfo->ExceptionRecord->ExceptionInformation[1] << std::endl;
			}
			else if(ExceptionInfo->ExceptionRecord->ExceptionInformation[0] == 8)
			{
				// user-mode data execution prevention (DEP)
				report << "Data Execution Prevention (DEP) at: 0x" << std::hex << std::uppercase << std::setw(8) << std::setfill('0') << ExceptionInfo->ExceptionRecord->ExceptionInformation[1] << std::endl;
			}
			else
			{
				// unknown, shouldn't happen
				report << "Unknown access violation at: 0x" << std::hex << std::uppercase << std::setw(8) << std::setfill('0') << ExceptionInfo->ExceptionRecord->ExceptionInformation[1] << std::endl;
			}

			// log NTSTATUS
			report << "NTSTATUS: 0x" << std::hex << std::uppercase << std::setw(8) << std::setfill('0') << ExceptionInfo->ExceptionRecord->ExceptionInformation[2] << std::endl;
		}
		break;

		case EXCEPTION_INT_DIVIDE_BY_ZERO:
			report << "What: EXCEPTION_INT_DIVIDE_BY_ZERO" << std::endl;
			break;

		case EXCEPTION_INT_OVERFLOW:
			report << "What: EXCEPTION_INT_OVERFLOW" << std::endl;
			break;

		case EXCEPTION_INVALID_DISPOSITION:
			report << "What: EXCEPTION_INVALID_DISPOSITION" << std::endl;
			break;

		case EXCEPTION_NONCONTINUABLE_EXCEPTION:
			report << "What: EXCEPTION_NONCONTINUABLE_EXCEPTION" << std::endl;
			break;

		case EXCEPTION_PRIV_INSTRUCTION:
			report << "What: EXCEPTION_PRIV_INSTRUCTION" << std::endl;
			break;

		case EXCEPTION_SINGLE_STEP:
			report << "What: EXCEPTION_SINGLE_STEP" << std::endl;
			break;

		case EXCEPTION_STACK_OVERFLOW:
			report << "What: EXCEPTION_STACK_OVERFLOW" << std::endl;
			break;

		case DBG_CONTROL_C:
			report << "What: DBG_CONTROL_C" << std::endl;
			break;

		default:
			report << "What: 0x" << std::hex << std::uppercase << std::setw(8) << std::setfill('0') << ExceptionInfo->ExceptionRecord->ExceptionCode << std::endl;
	}

	report << std::endl << "Registers:" << std::endl << std::endl;
	report << "EAX: 0x" << std::hex << std::uppercase << std::setw(8) << std::setfill('0') << ExceptionInfo->ContextRecord->Eax << "  ||  ESI: 0x" << std::hex << std::uppercase << std::setw(8) << std::setfill('0') << ExceptionInfo->ContextRecord->Esi << std::endl;
	report << "EBX: 0x" << std::hex << std::uppercase << std::setw(8) << std::setfill('0') << ExceptionInfo->ContextRecord->Ebx << "  ||  EDI: 0x" << std::hex << std::uppercase << std::setw(8) << std::setfill('0') << ExceptionInfo->ContextRecord->Edi << std::endl;
	report << "ECX: 0x" << std::hex << std::uppercase << std::setw(8) << std::setfill('0') << ExceptionInfo->ContextRecord->Ecx << "  ||  EBP: 0x" << std::hex << std::uppercase << std::setw(8) << std::setfill('0') << ExceptionInfo->ContextRecord->Ebp << std::endl;
	report << "EDX: 0x" << std::hex << std::uppercase << std::setw(8) << std::setfill('0') << ExceptionInfo->ContextRecord->Edx << "  ||  ESP: 0x" << std::hex << std::uppercase << std::setw(8) << std::setfill('0') << ExceptionInfo->ContextRecord->Esp << std::endl;
	report << std::endl;
	report << "Backtrace:" << std::endl << std::endl;

	for(std::list<std::string>::iterator i = gDebug->funcTrace.begin(); i != gDebug->funcTrace.end(); i++)
	{
		if(i == gDebug->funcTrace.begin())
			report << (*i) << std::endl;
		else
			report << "<= " << (*i) << std::endl;
	}

	report << std::endl << std::endl;

	report.close();

	gDebug->Log("Exception 0x%08x at address 0x%08x. Addon was terminated. See 'addon_crashreport.log'", ExceptionInfo->ExceptionRecord->ExceptionCode, ExceptionInfo->ExceptionRecord->ExceptionAddress);

	boost::this_thread::sleep_for(boost::chrono::milliseconds(500));
	exit(EXIT_FAILURE);

	return EXCEPTION_CONTINUE_SEARCH;
}



void addonDebug::Log(char *format, ...)
{
	va_list args;

	va_start(args, format);

	try_lock_mutex:

	if(this->lwrMutex.try_lock())
	{
		this->logQueue.push(addonString::vprintf(format, args));
		this->lwrMutex.unlock();
	}
	else
	{
		boost::this_thread::sleep_for(boost::chrono::milliseconds(1));
		goto try_lock_mutex;
	}

	va_end(args);
}



void addonDebug::traceLastFunction(char *format, ...)
{
	va_list args;

	va_start(args, format);

	boost::unique_lock<boost::mutex> lockit(this->ftrMutex);
	this->funcTrace.push_front(addonString::vprintf(format, args));
	lockit.unlock();

	va_end(args);

	if(this->funcTrace.size() > 20)
	{
		lockit.lock();
		this->funcTrace.pop_back();
		lockit.unlock();
	}
}



void addonDebug::processFW()
{
	char timeform[16];
	struct tm *timeinfo;
	time_t rawtime;

	std::ofstream file;
	std::string data;

	while(true)
	{
		this->lwrMutex.lock();

		if(this->logQueue.empty())
		{
			this->lwrMutex.unlock();

			break;
		}

		data = this->logQueue.front();
		this->lwrMutex.unlock();

		time(&rawtime);
		timeinfo = localtime(&rawtime);
		strftime(timeform, sizeof timeform, "%X", timeinfo);

		file.open(".\\addon_log.txt", (std::ofstream::out | std::ofstream::app));
		file << "[" << timeform << "] " << data << std::endl;
		file.close();

		try_lock_mutex:

		if(this->lwrMutex.try_lock())
		{
			this->logQueue.pop();
			this->lwrMutex.unlock();
		}
		else
		{
			gDebug->Log("Cannot lock debug queue mutex, continuing anyway");
			boost::this_thread::sleep_for(boost::chrono::milliseconds(1));
			goto try_lock_mutex;
		}

		boost::this_thread::sleep_for(boost::chrono::milliseconds(1));
	}
}



void addonDebug::Thread()
{
	assert(gDebug->getThreadInstance()->get_id() == boost::this_thread::get_id());

	gDebug->traceLastFunction("addonDebug::Thread() at 0x%x", &addonDebug::Thread);
	//gDebug->Log("Started file debug thread with id 0x%x", gDebug->getThreadInstance()->get_thread_info()->id);

	while(true)
	{
		boost::this_thread::disable_interruption di;

		gDebug->processFW();

		boost::this_thread::restore_interruption re(di);
		boost::this_thread::sleep_for(boost::chrono::milliseconds(50));
	}
}