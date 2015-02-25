#pragma once



#include "client.hpp"





class addonTransfer
{

public:

	addonTransfer(boost::asio::ip::tcp::socket& sock);
	virtual ~addonTransfer();

	void sendFile(std::string filename);
	void recvFile(std::string filename, std::size_t remote_file_size, int remote_file_crc);
	
	void sendFileThread(std::string filename, std::size_t file_size, int file_crc);
	void recvFileThread(std::string filename, std::size_t remote_file_size, int remote_file_crc);

	bool isTransfering()
	{
		boost::shared_lock<boost::shared_mutex> lockit(tMutex);
		return transfering;
	}

private:

	bool transfering;

	boost::shared_ptr<boost::thread> tHandle;
	boost::shared_mutex tMutex;
	boost::asio::ip::tcp::socket& s;
};