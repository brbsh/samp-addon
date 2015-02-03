#pragma once



#include "client.h"





class addonLoader
{

public:

	addonLoader();
	virtual ~addonLoader();

	boost::mutex *getMutexInstance();
	boost::thread *getThreadInstance();

	std::size_t crc32_file(std::string filename)
	{
		boost::crc_32_type result;
		std::ifstream i;

		i.open(filename, std::fstream::binary);
		
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

private:

	boost::shared_ptr<boost::mutex> mutexInstance;
	boost::shared_ptr<boost::thread> threadInstance;
};