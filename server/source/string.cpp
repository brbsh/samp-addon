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
	int len;
	cell *ptr;

	amx_GetAddr(amx, param, &ptr);
	amx_StrLen(ptr, &len);

	char *dest = new char[(len + 1)];

	amx_GetString(dest, ptr, NULL, UNLIMITED);
	dest[len] = '\0';

	std::string ret(dest);

	delete[] dest;

	return ret;
}