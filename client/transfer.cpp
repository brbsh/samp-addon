#pragma once



#include "transfer.h"





addonTransfer *gTransfer;

//extern addonMutex *gMutex;
extern addonSocket *gSocket;





void addonTransfer::SendThread(int socketid)
{
	boost::this_thread::sleep(boost::posix_time::seconds(3));

	addonDebug("Thread addonTransfer::SendThread(%i) started", socketid);

	char buffer[256];

	FILE *io;
	long length;
	std::stringstream format;

	fopen_s(&io, gSocket->Transfer.file.c_str(), "rb");
	fseek(io, NULL, SEEK_END);

	format << "TCPQUERY CLIENT_CALL" << " " << 2001 << " " << gSocket->Transfer.file << " " << ftell(io);
	send(socketid, format.str().c_str(), format.str().length(), NULL);

	recv(socketid, buffer, sizeof buffer, NULL);

	if(strcmp(buffer, "TCPQUERY SERVER_CALL 2001 READY"))
	{
		addonDebug("Error!");

		fclose(io);

		return;
	}

	fseek(io, NULL, SEEK_SET);

	while(!feof(io))
	{
		length = fread(buffer, 1, sizeof buffer, io);
		send(socketid, buffer, length, NULL);

		boost::this_thread::sleep(boost::posix_time::milliseconds(100));
	}

	send(socketid, "TCPQUERY CLIENT_CALL 2001 END", 29, NULL);

	recv(socketid, buffer, sizeof buffer, NULL);

	if(strcmp(buffer, "TCPQUERY SERVER_CALL 2001 END"))
	{
		addonDebug("Error");

		fclose(io);

		return;
	}

	fclose(io);

	boost::mutex::scoped_lock lock(gSocket->Mutex);
	gSocket->Transfer.is = false;
	lock.unlock();
}



void addonTransfer::ReceiveThread(int socketid)
{
	boost::this_thread::sleep(boost::posix_time::seconds(3));

	char buffer[256];

	addonDebug("Thread addonTransfer::ReceiveThread(%i) started", socketid);

	FILE *io;
	long length;

	fopen_s(&io, gSocket->Transfer.file.c_str(), "ab");

	send(socketid, "TCPQUERY CLIENT_CALL 2000 READY", 31, NULL);

	while(true)
	{
		length = recv(socketid, buffer, sizeof buffer, NULL);
		fwrite(buffer, 1, length, io);

		if(ftell(io) >= gSocket->Transfer.file_length)
			break;

		boost::this_thread::sleep(boost::posix_time::milliseconds(100));
	}

	fclose(io);

	recv(socketid, buffer, sizeof buffer, NULL);

	if(strcmp(buffer, "TCPQUERY SERVER_CALL 2000 END"))
	{
		addonDebug("No end header");

		return;
	}

	send(socketid, "TCPQUERY CLIENT_CALL 2000 END", 29, NULL);

	boost::mutex::scoped_lock lock(gSocket->Mutex);
	gSocket->Transfer.is = false;
	lock.unlock();
}