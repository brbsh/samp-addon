#pragma once



#include "string.h"





extern boost::shared_ptr<addonDebug> gDebug;





std::string addonString::wstringToString(const std::wstring &input)
{
	gDebug->TraceLastFunction(strFormat() << "addonString::wstringToString(...) at 0x" << std::hex << &addonString::wstringToString);

	char *buffer = new char[(input.length() + 1)];
	std::string output;

	buffer[input.length()] = NULL;
	WideCharToMultiByte(CP_ACP, NULL, input.c_str(), -1, buffer, static_cast<int>(input.length()), NULL, NULL);
	output.assign(buffer);

	delete[] buffer;

	return output;
}



std::string addonString::vprintf(const char *format, va_list args)
{
	gDebug->TraceLastFunction(strFormat() << "addonString::vprintf(...) at 0x" << std::hex << &addonString::vprintf);

	int length = vsnprintf(NULL, NULL, format, args);
	char *chars = new char[++length];

	vsnprintf(chars, length, format, args);
	std::string result(chars);
	
	delete[] chars;

	return result;
}