#pragma once



#include "string.h"





addonString *gString;





std::string addonString::wstring_to_string(const std::wstring &input)
{
	char *buffer = (char *)malloc(input.length() + 1);
	std::string output;

	buffer[input.length()] = NULL;
	WideCharToMultiByte(CP_ACP, NULL, input.c_str(), -1, buffer, static_cast<int>(input.length()), NULL, NULL);
	output.assign(buffer);

	free(buffer);

	return output;
}
