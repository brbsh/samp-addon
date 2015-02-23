#pragma once



#include "client.hpp"





namespace addonHash
{
	int crc32(std::string data, std::size_t size);
	int crc32_file(std::string filename);
}