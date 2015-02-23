#pragma once



#include "client.hpp"





extern boost::shared_ptr<addonDebug> gDebug;





std::string addonString::vprintf(const char *format, va_list args)
{
	//gDebug->traceLastFunction("addonString::vprintf(...) at 0x%x", &addonString::vprintf);

	int length = vsnprintf(NULL, NULL, format, args);
	char *chars = new char[++length];

	vsnprintf(chars, length, format, args);
	std::string result(chars);
	
	delete[] chars;

	return result;
}