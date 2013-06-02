#pragma once


#include "string.h"





std::string addonString::wstring_to_string(const std::wstring &input)
{
	char *buffer = new char[input.length() + 1];
	buffer[input.size()] = '\0';
	WideCharToMultiByte(CP_ACP, 0, input.c_str(), -1, buffer, static_cast<int>(input.length()), NULL, NULL);
	std::string output;
	output = buffer;
	delete[] buffer;
	return output;
}