#pragma once



#include "transfer.h"





addonTransfer *gTransfer;

extern addonMutex *gMutex;
extern addonSocket *gSocket;





DWORD addonTransfer::SendThread(void *lpParam)
{
	Sleep(2500);

	int socketid = (int)lpParam;

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

		goto Error;
	}

	fseek(io, NULL, SEEK_SET);

	while(!feof(io))
	{
		length = fread(buffer, 1, sizeof buffer, io);
		send(socketid, buffer, length, NULL);

		Sleep(100);
	}

	send(socketid, "TCPQUERY CLIENT_CALL 2001 END", 29, NULL);

	recv(socketid, buffer, sizeof buffer, NULL);

	if(strcmp(buffer, "TCPQUERY SERVER_CALL 2001 END"))
	{
		addonDebug("Error");
	}

Error:

	fclose(io);

	gMutex->Lock();
	gSocket->Transfer.is = false;
	gMutex->unLock();

	return true;
}



DWORD addonTransfer::ReceiveThread(void *lpParam)
{
	Sleep(2500);

	int socketid = (int)lpParam;
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

		Sleep(100);
	}

	fclose(io);

	recv(socketid, buffer, sizeof buffer, NULL);

	if(strcmp(buffer, "TCPQUERY SERVER_CALL 2000 END"))
	{
		addonDebug("No end header");
	}

	send(socketid, "TCPQUERY CLIENT_CALL 2000 END", 29, NULL);

	gMutex->Lock();
	gSocket->Transfer.is = false;
	gMutex->unLock();

	return true;
}