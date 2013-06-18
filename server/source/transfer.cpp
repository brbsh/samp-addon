#pragma once



#include "transfer.h"





amxTransfer *gTransfer;


extern logprintf_t logprintf;

extern amxMutex *gMutex;
extern amxPool *gPool;

extern std::queue<amxFileReceive> amxFileReceiveQueue;
extern std::queue<amxFileSend> amxFileSendQueue;
extern std::queue<amxFileError> amxFileErrorQueue;





#ifdef WIN32
	DWORD amxTransfer::SendThread(void *lpParam)
#else
	void *amxTransfer::SendThread(void *lpParam)
#endif
{
	SLEEP(2500);

	int clientid = (int)lpParam;
	int socketid;

	logprintf("Thread amxTransfer::SendThread(%i) started", clientid);

	char buffer[256];
	
	FILE *io;
	long length;
	std::stringstream format;
	
	sockPool pool;
	amxFileSend pushme;
	amxFileError pushError;

	socketid = gPool->socketPool[clientid].socketid;

	fopen_s(&io, gPool->transferPool[clientid].file.c_str(), "rb");

	if(!io)
	{
		pushError.io = 0;
		pushError.clientid = clientid;
		pushError.file = gPool->transferPool[clientid].file;
		pushError.error.assign("Cannot read from file");
		pushError.errorCode = 1;

		gMutex->Lock();
		amxFileErrorQueue.push(pushError);
		gMutex->unLock();

		goto Error;
	}

	fseek(io, NULL, SEEK_END);

	format << "TCPQUERY SERVER_CALL" << " " << 2000 << " " << gPool->transferPool[clientid].remote_file << " " << ftell(io);
	send(socketid, format.str().c_str(), format.str().length(), NULL);

	recv(socketid, (char *)&buffer, sizeof buffer, NULL);

	if(strcmp(buffer, "TCPQUERY CLIENT_CALL 2000 READY"))
	{
		pushError.io = 0;
		pushError.clientid = clientid;
		pushError.file = gPool->transferPool[clientid].file;
		pushError.error.assign("Client isn't ready for file transfering");
		pushError.errorCode = 2;

		gMutex->Lock();
		amxFileErrorQueue.push(pushError);
		gMutex->unLock();

		goto Error;
	}

	fseek(io, NULL, SEEK_SET);

	while(!feof(io))
	{
		length = fread(buffer, 1, sizeof buffer, io);
		send(socketid, buffer, length, NULL);

		SLEEP(100);
	}

	send(socketid, "TCPQUERY SERVER_CALL 2000 END", 29, NULL);

	recv(socketid, buffer, sizeof buffer, NULL);

	if(strcmp(buffer, "TCPQUERY CLIENT_CALL 2000 END"))
	{
		pushError.io = 0;
		pushError.clientid = clientid;
		pushError.file = gPool->transferPool[clientid].file;
		pushError.error.assign("Invalid final response from client");
		pushError.errorCode = 3;

		gMutex->Lock();
		amxFileErrorQueue.push(pushError);
		gMutex->unLock();

		goto Error;
	}

	pushme.clientid = clientid;
	pushme.file = gPool->transferPool[clientid].remote_file;

	gMutex->Lock();
	amxFileSendQueue.push(pushme);
	gMutex->unLock();

Error:
	
	gMutex->Lock();
	gPool->transferPool.erase(clientid);
	pool = gPool->socketPool[clientid];
	gMutex->unLock();

	pool.transfer = false;

	gMutex->Lock();
	gPool->socketPool[clientid] = pool;
	gMutex->unLock();

	fclose(io);

	#ifdef WIN32
		return true;
	#endif
}



#ifdef WIN32
	DWORD amxTransfer::ReceiveThread(void *lpParam)
#else
	void *amxTransfer::ReceiveThread(void *lpParam)
#endif
{
	SLEEP(2500);

	int clientid = (int)lpParam;
	int socketid;

	char buffer[256];

	logprintf("Thread amxTransfer::ReceiveThread(%i) started", clientid);

	FILE *io;
	long length;

	sockPool pool;
	amxFileReceive pushme;
	amxFileError pushError;

	socketid = gPool->socketPool[clientid].socketid;
	
	fopen_s(&io, gPool->transferPool[clientid].file.c_str(), "ab");

	if(!io)
	{
		pushError.io = 1;
		pushError.clientid = clientid;
		pushError.file = gPool->transferPool[clientid].file;
		pushError.error.assign("Cannot write/append to file");
		pushError.errorCode = 1;

		gMutex->Lock();
		amxFileErrorQueue.push(pushError);
		gMutex->unLock();

		goto Error;
	}

	send(socketid, "TCPQUERY SERVER_CALL 2001 READY", 31, NULL);

	while(true)
	{
		length = recv(socketid, buffer, sizeof buffer, NULL);
		fwrite(buffer, 1, length, io);

		if(ftell(io) >= gPool->transferPool[clientid].file_length)
			break;

		SLEEP(100);
	}

	fclose(io);

	recv(socketid, buffer, sizeof buffer, NULL);

	if(strcmp(buffer, "TCPQUERY CLIENT_CALL 2001 END"))
	{
		pushError.io = 1;
		pushError.clientid = clientid;
		pushError.file = gPool->transferPool[clientid].file;
		pushError.error.assign("Invalid end flag from client");
		pushError.errorCode = 2;

		gMutex->Lock();
		amxFileErrorQueue.push(pushError);
		gMutex->unLock();

		goto Error;
	}

	send(socketid, "TCPQUERY SERVER_CALL 2001 END", 29, NULL);

	pushme.clientid = clientid;
	pushme.file = gPool->transferPool[clientid].file;

	gMutex->Lock();
	amxFileReceiveQueue.push(pushme);
	gMutex->unLock();

Error:

	gMutex->Lock();
	pool = gPool->socketPool[clientid];
	gPool->transferPool.erase(clientid);
	gMutex->unLock();

	pool.transfer = false;

	gMutex->Lock();
	gPool->socketPool[clientid] = pool;
	gMutex->unLock();
	
	#ifdef WIN32
		return true;
	#endif
}