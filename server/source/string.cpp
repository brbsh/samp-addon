#pragma once



#include "string.h"



amxString *gString;





void amxString::Set(AMX *amx, cell param, std::string data)
{
	cell *destination;

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