#pragma once



#include "client.hpp"





boost::shared_ptr<addonTransfer> gTransfer;


extern boost::shared_ptr<addonDebug> gDebug;





addonTransfer::addonTransfer(boost::asio::ip::tcp::socket& sock) : s(sock)
{
	gDebug->Log("Transfer constructor called");

	transfering = false;
}



addonTransfer::~addonTransfer()
{
	gDebug->Log("Transfer destructor called");

	rcvThread->interrupt();
}



void addonTransfer::recvFile(std::string filename, std::size_t remote_file_size, int remote_file_crc)
{
	boost::upgrade_lock<boost::shared_mutex> lockit(tMutex);
	
	if(transfering)
	{
		return;
		// already started
	}

	boost::upgrade_to_unique_lock<boost::shared_mutex> lockit_(lockit);
	transfering = true;

	rcvThread = boost::shared_ptr<boost::thread>(new boost::thread(boost::bind(&addonTransfer::recvFileThread, this, filename, remote_file_size, remote_file_crc)));
}



void addonTransfer::recvFileThread(std::string filename, std::size_t remote_file_size, int remote_file_crc)
{
	/*std::string query = boost::str(boost::format("%1%") % remote_file_crc);

	try
	{
		s.write_some(boost::asio::buffer(query, query.length()));
	}
	catch(boost::system::system_error& error)
	{
		gDebug->Log("Error while sending hashcheck for file transfer: %s", error.what());
	}*/

	std::ofstream file(filename.c_str(), (std::ofstream::out | std::ofstream::binary));
	std::size_t bytes_rx = NULL;

	char *block = new char[remote_file_size + 1];
	memset(block, NULL, remote_file_size);

	try
	{
		bytes_rx = boost::asio::read(s, boost::asio::buffer(block, remote_file_size), boost::asio::transfer_exactly(remote_file_size));

		file.write(block, bytes_rx);
		file.close();

		delete[] block;
	}
	catch(boost::system::system_error& error)
	{
		file.close();
		delete[] block;

		boost::unique_lock<boost::shared_mutex> lockit(tMutex);
		transfering = false;

		gDebug->Log("File %s transfer error: %s", filename.c_str(), error.what());

		return;
	}

	int file_crc = addonHash::crc32_file(filename);

	if(file_crc == remote_file_crc)
	{
		gDebug->Log("File %s transfer completed (CRC: %i)", filename.c_str(), file_crc);
	}
	else
	{
		gDebug->Log("File %s transfer error (Final CRC check fail [L:%i != R:%i])", filename.c_str(), file_crc, remote_file_crc);
	}

	gDebug->Log("File transfer cycle completed");

	boost::unique_lock<boost::shared_mutex> lockit(tMutex);
	transfering = false;
}