#pragma once



#include "addon.h"





class addonHash
{

public:

	unsigned int MurmurHash2(char *key, unsigned int len);
};