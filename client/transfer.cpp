#pragma once



#include "transfer.h"





addonTransfer *gTransfer;


extern addonData *gData;
extern addonSocket *gSocket;





void addonTransfer::SendThread(std::string file_name)
{
	char buffer[1024];

	FILE *io;

	boost::mutex sMutex;
	std::stringstream format;
	std::size_t file_length = NULL;
	std::size_t length = NULL;
	std::size_t total = NULL;

	addonDebug("Thread addonTransfer::SendThread(%s) started", file_name.c_str());
	
	fopen_s(&io, file_name.c_str(), "rb");

	if(!io)
	{
		addonDebug("Cannot read from file %s", file_name.c_str());

		return;
	}

	addonDebug("File %s opened in read/binary mode", file_name.c_str());

	fseek(io, NULL, SEEK_END);

	file_length = (unsigned int)ftell(io);

	addonDebug("File %s has %i bytes length", file_name.c_str(), file_length);

	format << "TCPQUERY CLIENT_CALL " << 2001 << " " << file_name << " " << file_length;
	gSocket->Socket->write_some(boost::asio::buffer(format.str()));

	addonDebug("Length query sended, sleeping for 1 sec...");

	boost::this_thread::sleep(boost::posix_time::seconds(1));

	gSocket->Socket->read_some(boost::asio::buffer(buffer, sizeof buffer));

	if(strcmp(buffer, "TCPQUERY SERVER_CALL 2001 START"))
	{
		addonDebug("Error1!");

		boost::mutex::scoped_lock lock(sMutex);
		gData->Transfer.Active = false;
		lock.unlock();

		fclose(io);

		return;
	}

	gSocket->Socket->write_some(boost::asio::buffer("TCPQUERY CLIENT_CALL 2001 START", 32));

	boost::this_thread::sleep(boost::posix_time::seconds(1));

	gSocket->Socket->read_some(boost::asio::buffer(buffer, sizeof buffer));

	if(strcmp(buffer, "TCPQUERY SERVER_CALL 2001 READY"))
	{
		addonDebug("Error2!");

		fclose(io);

		return;
	}

	addonDebug("Server is ready for file transfering, processing...");

	fseek(io, NULL, SEEK_SET);

	while(total < file_length)
	{
		length = fread(buffer, 1, sizeof buffer, io);
		total += length;

		if(total > file_length)
		{
			addonDebug("WARNING: Total transfered data (%i) bigger than it's length (%i), cutted", total, file_length);
			gSocket->Socket->write_some(boost::asio::buffer(buffer, (length - (total - file_length))));
		}
		else
			gSocket->Socket->write_some(boost::asio::buffer(buffer, length));

		boost::this_thread::sleep(boost::posix_time::milliseconds(50));
	}

	addonDebug("Waiting for server accept transfer query...");

	gSocket->Socket->read_some(boost::asio::buffer(buffer, sizeof buffer));

	if(strcmp(buffer, "TCPQUERY SERVER_CALL 2001 END"))
	{
		addonDebug("Error");

		boost::mutex::scoped_lock lock(sMutex);
		gData->Transfer.Active = false;
		lock.unlock();

		fclose(io);

		return;
	}

	addonDebug("File %s was successfuly sent to server", file_name.c_str());

	fclose(io);

	boost::mutex::scoped_lock lock(sMutex);
	gData->Transfer.Active = false;
	lock.unlock();
}



void addonTransfer::ReceiveThread(std::string file_name, std::size_t file_length)
{
	char buffer[1024];

	FILE *io;

	boost::mutex rMutex;
	boost::system::error_code error;
	std::size_t length = NULL;
	std::size_t total = NULL;

	addonDebug("Thread addonTransfer::ReceiveThread(%s, %i) started", file_name.c_str(), file_length);

	fopen_s(&io, file_name.c_str(), "wb");

	if(!io)
	{
		addonDebug("Cannot write/append to file");

		return;
	}

	addonDebug("File %s opened in append/binary mode", file_name.c_str());

	gSocket->Socket->write_some(boost::asio::buffer("TCPQUERY CLIENT_CALL 2000 READY", 32));

	addonDebug("Started file receiving from server...");

	while(total < file_length)
	{
		length = gSocket->Socket->read_some(boost::asio::buffer(buffer, sizeof buffer), error);

		if(error)
		{
			addonDebug("Error while transfering file %s", file_name.c_str());

			return;
		}

		if(length > 0)
		{
			total += length;

			if(total > file_length)
			{
				addonDebug("WARNING: Total transfered data (%i) bigger than it's length (%i), cutted", total, file_length);
				fwrite(buffer, 1, (length - (total - file_length)), io);
			}
			else
				fwrite(buffer, 1, length, io);
		}

		boost::this_thread::sleep(boost::posix_time::milliseconds(50));
	}

	fclose(io);

	/*gSocket->Socket->read_some(boost::asio::buffer(buffer, sizeof buffer));

	if(strcmp(buffer, "TCPQUERY SERVER_CALL 2000 END"))
	{
		addonDebug("No end header");

		return;
	}*/

	gSocket->Socket->write_some(boost::asio::buffer("TCPQUERY CLIENT_CALL 2000 END", 30));

	addonDebug("File %s was successfuly received from server", file_name.c_str());

	boost::mutex::scoped_lock lock(rMutex);
	gData->Transfer.Active = false;
	lock.unlock();
}