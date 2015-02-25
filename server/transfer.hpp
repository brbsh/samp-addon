#pragma once



#include "server.hpp"





class amxTransfer
{

public:

	amxTransfer(bool TLF, unsigned int clientid, std::string filename, std::string remote_filename, std::size_t remote_file_size, int remote_file_crc);
	virtual ~amxTransfer();

	void processSend(unsigned int clientid, const boost::system::error_code& error, std::size_t bytes_tx);
	void processRecv(unsigned int clientid, const char *buffer, const boost::system::error_code& error, std::size_t bytes_rx);

	bool isLocal()
	{
		return is_local;
	}

	int fileCRC()
	{
		return file_crc;
	}

private:

	std::fstream file;
	std::string file_name;
	std::string remote_file_name;
	std::size_t file_size;
	int file_crc;
	char *file_buf;
	bool is_local;
};