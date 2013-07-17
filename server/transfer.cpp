#pragma once



#include "transfer.h"





amxTransfer *gTransfer;


extern logprintf_t logprintf;

extern amxPool *gPool;

extern std::queue<amxFileReceive> amxFileReceiveQueue;
extern std::queue<amxFileSend> amxFileSendQueue;
extern std::queue<amxFileError> amxFileErrorQueue;





void amxTransfer::SendThread(int clientid)
{
	/*boost::mutex tMutex;

	boost::this_thread::sleep(boost::posix_time::seconds(3));

	int socketid;

	logprintf("Thread amxTransfer::SendThread(%i) started", clientid);

	char buffer[256];
	
	FILE *io;
	long length;
	std::stringstream format;
	
	sockPool pool;
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

		boost::mutex::scoped_lock lock(gTransfer->Mutex);
		amxFileErrorQueue.push(pushError);
		lock.unlock();

		boost::mutex::scoped_lock pool_lock(gPool->Mutex);
		gPool->transferPool.erase(clientid);
		pool = gPool->socketPool[clientid];

		pool.transfer = false;

		gPool->socketPool[clientid] = pool;
		pool_lock.unlock();

		return;
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

		boost::mutex::scoped_lock lock(gTransfer->Mutex);
		amxFileErrorQueue.push(pushError);
		lock.unlock();

		boost::mutex::scoped_lock pool_lock(gPool->Mutex);
		gPool->transferPool.erase(clientid);
		pool = gPool->socketPool[clientid];

		pool.transfer = false;

		gPool->socketPool[clientid] = pool;
		pool_lock.unlock();

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

	send(socketid, "TCPQUERY SERVER_CALL 2000 END", 29, NULL);

	recv(socketid, buffer, sizeof buffer, NULL);

	if(strcmp(buffer, "TCPQUERY CLIENT_CALL 2000 END"))
	{
		pushError.io = 0;
		pushError.clientid = clientid;
		pushError.file = gPool->transferPool[clientid].file;
		pushError.error.assign("Invalid final response from client");
		pushError.errorCode = 3;

		boost::mutex::scoped_lock lock(gTransfer->Mutex);
		amxFileErrorQueue.push(pushError);
		lock.unlock();

		boost::mutex::scoped_lock pool_lock(gPool->Mutex);
		gPool->transferPool.erase(clientid);
		pool = gPool->socketPool[clientid];

		pool.transfer = false;

		gPool->socketPool[clientid] = pool;
		pool_lock.unlock();

		fclose(io);

		return;
	}

	amxFileSend pushme;

	pushme.clientid = clientid;
	pushme.file = gPool->transferPool[clientid].remote_file;

	boost::mutex::scoped_lock lock(gTransfer->Mutex);
	amxFileSendQueue.push(pushme);
	lock.unlock();

	fclose(io);*/
}



void amxTransfer::ReceiveThread(int clientid)
{
	/*boost::this_thread::sleep(boost::posix_time::seconds(3));

	int socketid;
	char buffer[256];

	logprintf("Thread amxTransfer::ReceiveThread(%i) started", clientid);

	FILE *io;
	long length;

	sockPool pool;
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

		boost::mutex::scoped_lock lock(gTransfer->Mutex);
		amxFileErrorQueue.push(pushError);
		lock.unlock();

		boost::mutex::scoped_lock pool_lock(gPool->Mutex);
		pool = gPool->socketPool[clientid];
		gPool->transferPool.erase(clientid);

		pool.transfer = false;

		gPool->socketPool[clientid] = pool;
		lock.unlock();

		return;
	}

	send(socketid, "TCPQUERY SERVER_CALL 2001 READY", 31, NULL);

	while(true)
	{
		length = recv(socketid, buffer, sizeof buffer, NULL);
		fwrite(buffer, 1, length, io);

		if(ftell(io) >= gPool->transferPool[clientid].file_length)
			break;

		boost::this_thread::sleep(boost::posix_time::milliseconds(100));
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

		boost::mutex::scoped_lock lock(gTransfer->Mutex);
		amxFileErrorQueue.push(pushError);
		lock.unlock();

		boost::mutex::scoped_lock pool_lock(gPool->Mutex);
		pool = gPool->socketPool[clientid];
		gPool->transferPool.erase(clientid);

		pool.transfer = false;

		gPool->socketPool[clientid] = pool;
		lock.unlock();

		return;
	}

	send(socketid, "TCPQUERY SERVER_CALL 2001 END", 29, NULL);

	amxFileReceive pushme;

	pushme.clientid = clientid;
	pushme.file = gPool->transferPool[clientid].file;

	boost::mutex::scoped_lock lock(gTransfer->Mutex);
	amxFileReceiveQueue.push(pushme);
	lock.unlock();

	boost::mutex::scoped_lock pool_lock(gPool->Mutex);
	pool = gPool->socketPool[clientid];
	gPool->transferPool.erase(clientid);

	pool.transfer = false;

	gPool->socketPool[clientid] = pool;
	lock.unlock();*/
}