#pragma once



#include "client.hpp"





boost::shared_ptr<addonTransfer> gTransfer;


extern boost::shared_ptr<addonDebug> gDebug;
extern boost::shared_ptr<addonSocket> gSocket;





addonTransfer::addonTransfer(boost::asio::ip::tcp::socket& sock) : s(sock)
{
	gDebug->Log("Transfer constructor called");

	transfering = false;
}



addonTransfer::~addonTransfer()
{
	gDebug->Log("Transfer destructor called");

	tHandle->interrupt();
}



void addonTransfer::sendFile(std::string filename)
{
	boost::upgrade_lock<boost::shared_mutex> lockit(tMutex);
	boost::filesystem::path fpath(filename);

	if(transfering)
	{
		return;
		//already started
	}

	if(!boost::filesystem::exists(fpath))
	{
		// file does not exist;

		return;
	}

	std::size_t file_size = boost::filesystem::file_size(fpath);
	int file_crc = addonHash::crc32_file(filename);

	gSocket->writeTo(boost::str(boost::format("CMDRESPONSE|%1%|%2%|%3%|%4%") % ADDON_CMD_QUERY_TRF % filename % file_size % file_crc));

	{
		boost::upgrade_to_unique_lock<boost::shared_mutex> lockit_(lockit);
		transfering = true;
	}

	tHandle = boost::shared_ptr<boost::thread>(new boost::thread(boost::bind(&addonTransfer::sendFileThread, this, filename, file_size, file_crc)));
}



void addonTransfer::recvFile(std::string filename, std::size_t remote_file_size, int remote_file_crc)
{
	boost::upgrade_lock<boost::shared_mutex> lockit(tMutex);
	
	if(transfering)
	{
		return;
		// already started
	}
	else
	{
		boost::upgrade_to_unique_lock<boost::shared_mutex> lockit_(lockit);
		transfering = true;
	}

	tHandle = boost::shared_ptr<boost::thread>(new boost::thread(boost::bind(&addonTransfer::recvFileThread, this, filename, remote_file_size, remote_file_crc)));
}



void addonTransfer::sendFileThread(std::string filename, std::size_t file_size, int file_crc)
{
	boost::this_thread::sleep(boost::posix_time::seconds(1));

	std::ifstream file(filename.c_str(), (std::ifstream::in | std::ifstream::binary));
	std::size_t bytes_tx = NULL;

	char *block = new char[file_size + 1];
	memset(block, NULL, file_size);

	try
	{
		file.seekg(std::ifstream::beg);
		file.read(block, file_size);
		bytes_tx = boost::asio::write(s, boost::asio::buffer(block, file_size), boost::asio::transfer_exactly(file_size));

		file.close();
		delete[] block;
	}
	catch(boost::system::system_error& error)
	{
		file.close();
		delete[] block;

		boost::unique_lock<boost::shared_mutex> lockit(tMutex);
		transfering = false;

		gDebug->Log("Remote file %s transfer error: %s", filename.c_str(), error.what());

		return;
	}

	if(bytes_tx == file_size)
	{
		gDebug->Log("Remote file transfer completed (%i bytes transfered)", bytes_tx);
	}
	else
	{
		gDebug->Log("Remote file transfer error (%i bytes of %i processed)", bytes_tx, file_size);
	}

	gDebug->Log("Remote file transfer cycle completed");

	boost::unique_lock<boost::shared_mutex> lockit(tMutex);
	transfering = false;
}



void addonTransfer::recvFileThread(std::string filename, std::size_t remote_file_size, int remote_file_crc)
{
	boost::this_thread::sleep(boost::posix_time::seconds(1));

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

		gDebug->Log("Local file %s transfer error: %s", filename.c_str(), error.what());

		return;
	}

	int file_crc = addonHash::crc32_file(filename);

	if(file_crc == remote_file_crc)
	{
		gDebug->Log("Local file %s transfer completed (CRC: %i)", filename.c_str(), file_crc);
	}
	else
	{
		gDebug->Log("Local file %s transfer error (Final CRC check fail [L:%i != R:%i])", filename.c_str(), file_crc, remote_file_crc);
	}

	gDebug->Log("Local file transfer cycle completed");

	boost::unique_lock<boost::shared_mutex> lockit(tMutex);
	transfering = false;
}