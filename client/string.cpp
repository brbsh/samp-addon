#pragma once



#include "string.h"





addonString *gString;





std::string addonString::wstring_to_string(const std::wstring &input)
{
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
	int length = vsnprintf(NULL, NULL, format, args);
	char *chars = new char[++length];

	vsnprintf(chars, length, format, args);
	std::string result(chars);
	
	delete[] chars;

	return result;
}