#pragma once



#include "server.hpp"





void amxString::Set(AMX *amx, cell param, std::string data)
{
	cell *destination = NULL;

	amx_GetAddr(amx, param, &destination);
	amx_SetString(destination, data.c_str(), NULL, NULL, data.length());
}



std::string amxString::Get(AMX *amx, cell param)
{
	char *dest = NULL;

	amx_StrParam(amx, param, dest);
	std::string ret(dest);

	return ret;
}



std::string amxString::vprintf(const char *format, va_list args)
{
	int length = vsnprintf(NULL, NULL, format, args);
	char *chars = new char[++length];

	vsnprintf(chars, length, format, args);
	std::string result(chars);
	
	delete[] chars;

	return result;
}