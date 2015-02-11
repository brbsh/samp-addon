#pragma once



#include "server.hpp"





namespace amxHash
{
	int crc32(std::string data, std::size_t size);
	int crc32_file(std::string filename);
}