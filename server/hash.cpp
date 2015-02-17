#pragma once



#include "server.hpp"





int amxHash::crc32(std::string data, std::size_t size)
{
	boost::crc_32_type result;

	char *buffer = new char[size + 1];

	strcpy(buffer, data.c_str());
	result.process_bytes(buffer, size);

	delete[] buffer;

	return result.checksum();
}



int amxHash::crc32_file(std::string filename)
{
	boost::crc_32_type result;
	std::ifstream i;

	i.open(filename.c_str(), std::fstream::binary); // c_str() for g++ support
		
	do
	{
		char block[2048];
		
		i.read(block, sizeof block);
		result.process_bytes(block, i.gcount());
	}
	while(i);

	i.close();

	return result.checksum();
}