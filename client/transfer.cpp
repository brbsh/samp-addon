#pragma once



#include "transfer.h"





addonTransfer *gTransfer;


extern addonData *gData;
extern addonSocket *gSocket;





void addonTransfer::SendThread(std::string file_name)
{
	boost::this_thread::sleep(boost::posix_time::seconds(1));

	addonDebug("Thread addonTransfer::SendThread() started");

	char buffer[1024];

	FILE *io;
	std::stringstream format;

	fopen_s(&io, file_name.c_str(), "rb");
	fseek(io, NULL, SEEK_END);

	format << "TCPQUERY CLIENT_CALL " << 2001 << " " << file_name << " " << ftell(io);

	gSocket->Socket->write_some(boost::asio::buffer(format.str(), format.str().length()));
	gSocket->Socket->read_some(boost::asio::buffer(buffer, sizeof buffer));

	if(strcmp(buffer, "TCPQUERY SERVER_CALL 2001 READY"))
	{
		addonDebug("Error!");

		fclose(io);

		return;
	}

	fseek(io, NULL, SEEK_SET);

	while(!feof(io))
	{
		gData->Transfer.Length = fread(buffer, 1, sizeof buffer, io);
		gSocket->Socket->write_some(boost::asio::buffer(buffer, gData->Transfer.Length));

		boost::this_thread::sleep(boost::posix_time::milliseconds(100));
	}

	gSocket->Socket->write_some(boost::asio::buffer("TCPQUERY CLIENT_CALL 2001 END", 29));
	gSocket->Socket->read_some(boost::asio::buffer(buffer, sizeof buffer));

	if(strcmp(buffer, "TCPQUERY SERVER_CALL 2001 END"))
	{
		addonDebug("Error");

		fclose(io);

		return;
	}

	fclose(io);

	boost::mutex::scoped_lock lock(gSocket->Mutex);
	gData->Transfer.Active = false;
	lock.unlock();
}



void addonTransfer::ReceiveThread(std::string file_name, long int file_length)
{
	boost::this_thread::sleep(boost::posix_time::seconds(1));

	char buffer[1024];

	addonDebug("Thread addonTransfer::ReceiveThread() started");

	FILE *io;
	fopen_s(&io, file_name.c_str(), "ab");

	gSocket->Socket->write_some(boost::asio::buffer("TCPQUERY CLIENT_CALL 2000 READY", 31));

	while(true)
	{
		gData->Transfer.Length = gSocket->Socket->read_some(boost::asio::buffer(buffer, sizeof buffer));
		fwrite(buffer, 1, gData->Transfer.Length, io);

		if(ftell(io) >= file_length)
			break;

		boost::this_thread::sleep(boost::posix_time::milliseconds(100));
	}

	fclose(io);

	gSocket->Socket->read_some(boost::asio::buffer(buffer, sizeof buffer));

	if(strcmp(buffer, "TCPQUERY SERVER_CALL 2000 END"))
	{
		addonDebug("No end header");

		return;
	}

	gSocket->Socket->write_some(boost::asio::buffer("TCPQUERY CLIENT_CALL 2000 END", 29));

	boost::mutex::scoped_lock lock(gSocket->Mutex);
	gData->Transfer.Active = false;
	lock.unlock();
}