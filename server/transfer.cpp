#pragma once



#include "transfer.h"





amxTransfer *gTransfer;


extern logprintf_t logprintf;

extern amxFS *gFS;
extern amxPool *gPool;
extern amxSocket *gSocket;

extern std::queue<amxFileReceive> amxFileReceiveQueue;
extern std::queue<amxFileSend> amxFileSendQueue;
extern std::queue<amxFileError> amxFileErrorQueue;





void amxTransfer::SendThread(int clientid, std::string file_name, std::string remote_file_name)
{
	char buffer[1024];

	FILE *io;

	cliPool cPool;
	amxFileError pushError;

	boost::mutex sMutex;
	boost::system::error_code error;
	boost::shared_ptr<boost::asio::ip::tcp::socket> socketid;
	std::stringstream format;
	std::size_t length = NULL;
	std::size_t total = NULL;
	std::size_t file_length = NULL;

	addonDebug("Thread amxTransfer::SendThread(%i, %s, %s) successfuly started", clientid, file_name.c_str(), remote_file_name.c_str());
	
	socketid = gPool->clientPool.find(clientid)->second.socketid;

	#if defined WIN32
		fopen_s(&io, file_name.c_str(), "rb");
	#else
		io = fopen(file_name.c_str(), "rb");
	#endif

	if(!io)
	{
		pushError.io = 0;
		pushError.clientid = clientid;
		pushError.file = file_name;
		pushError.error.assign("Cannot read from file");
		pushError.errorCode = 1;

		boost::mutex::scoped_lock lock(sMutex);
		amxFileErrorQueue.push(pushError);

		cPool = gPool->clientPool.find(clientid)->second;

		cPool.Transfer.Active = false;

		gPool->clientPool[clientid] = cPool;
		lock.unlock();

		return;
	}

	addonDebug("File %s opened in read/binary mode", file_name.c_str());

	fseek(io, NULL, SEEK_END);

	file_length = (unsigned int)ftell(io);

	addonDebug("File %s has %i bytes length", file_name.c_str(), file_length);

	format << "TCPQUERY SERVER_CALL" << " " << 2000 << " " << remote_file_name << " " << file_length;
	socketid->write_some(boost::asio::buffer(format.str()));

	addonDebug("Length query sended, sleeping for 1 sec...");

	boost::this_thread::sleep(boost::posix_time::seconds(1));

	socketid->write_some(boost::asio::buffer("TCPQUERY SERVER_CALL 2000 START", 32));
	socketid->read_some(boost::asio::buffer(buffer, sizeof buffer), error);

	if(error)
	{
		addonDebug("Error while receiving 'ready' header from %i", clientid);

		pushError.io = 0;
		pushError.clientid = clientid;
		pushError.file = file_name;
		pushError.error.assign("Error while receiving 'ready' header");
		pushError.errorCode = 2;

		boost::mutex::scoped_lock lock(sMutex);
		amxFileErrorQueue.push(pushError);

		cPool = gPool->clientPool.find(clientid)->second;

		cPool.Transfer.Active = false;

		gPool->clientPool[clientid] = cPool;
		lock.unlock();

		fclose(io);

		return;
	}

	if(strcmp(buffer, "TCPQUERY CLIENT_CALL 2000 READY"))
	{
		addonDebug("Client %i isn't ready for file transfering", clientid);

		pushError.io = 0;
		pushError.clientid = clientid;
		pushError.file = file_name;
		pushError.error.assign("Client isn't ready for file transfering");
		pushError.errorCode = 3;

		boost::mutex::scoped_lock lock(sMutex);
		amxFileErrorQueue.push(pushError);

		cPool = gPool->clientPool.find(clientid)->second;

		cPool.Transfer.Active = false;

		gPool->clientPool[clientid] = cPool;
		lock.unlock();

		fclose(io);

		return;
	}

	addonDebug("Client %i is ready for file transfer, processing...", clientid);

	fseek(io, NULL, SEEK_SET);

	while(total < file_length)
	{
		if(!gSocket->IsClientConnected(clientid))
			return;

		length = fread(buffer, 1, sizeof buffer, io);
		total += length;

		if(total > file_length)
		{
			addonDebug("WARNING: Total transfered data (%i) bigger than it's length (%i), cutted", total, file_length);
			socketid->write_some(boost::asio::buffer(buffer, (length - (total - file_length))));
		}
		else
			socketid->write_some(boost::asio::buffer(buffer, length));

		boost::this_thread::sleep(boost::posix_time::milliseconds(50));
	}

	addonDebug("Waiting for client %i accept transfer query...", clientid);

	socketid->read_some(boost::asio::buffer(buffer, sizeof buffer), error);

	if(error)
	{
		addonDebug("Error while receiving final response from client %i", clientid);

		pushError.io = 0;
		pushError.clientid = clientid;
		pushError.file = file_name;
		pushError.error.assign("Error while receiving final response");
		pushError.errorCode = 4;

		boost::mutex::scoped_lock lock(sMutex);
		amxFileErrorQueue.push(pushError);

		cPool = gPool->clientPool.find(clientid)->second;

		cPool.Transfer.Active = false;

		gPool->clientPool[clientid] = cPool;
		lock.unlock();

		fclose(io);

		return;
	}

	if(strcmp(buffer, "TCPQUERY CLIENT_CALL 2000 END"))
	{
		addonDebug("Invalid final response from client %i", clientid);

		pushError.io = 0;
		pushError.clientid = clientid;
		pushError.file = file_name;
		pushError.error.assign("Invalid final response from client");
		pushError.errorCode = 5;

		boost::mutex::scoped_lock lock(sMutex);
		amxFileErrorQueue.push(pushError);

		cPool = gPool->clientPool.find(clientid)->second;

		cPool.Transfer.Active = false;

		gPool->clientPool[clientid] = cPool;
		lock.unlock();

		fclose(io);

		return;
	}

	addonDebug("File %s was successfully sent to client %i", file_name.c_str(), clientid);

	amxFileSend pushme;

	pushme.clientid = clientid;
	pushme.file = remote_file_name;

	boost::mutex::scoped_lock lock(sMutex);
	amxFileSendQueue.push(pushme);

	cPool = gPool->clientPool.find(clientid)->second;

	cPool.Transfer.Active = false;

	gPool->clientPool[clientid] = cPool;
	lock.unlock();

	fclose(io);
}



void amxTransfer::ReceiveThread(int clientid, std::string file_name, std::size_t file_length)
{
	char buffer[1024];

	FILE *io;

	cliPool cPool;
	amxFileError pushError;

	boost::mutex rMutex;
	boost::system::error_code error;
	boost::shared_ptr<boost::asio::ip::tcp::socket> socketid;
	std::size_t length = NULL;
	std::size_t total = NULL;

	addonDebug("Thread amxTransfer::ReceiveThread(%i, %s, %i) started", clientid, file_name.c_str(), file_length);

	socketid = gPool->clientPool.find(clientid)->second.socketid;

	if(gFS->FileExist(file_name))
		gFS->RemoveFile(file_name);
	
	#if defined WIN32
		fopen_s(&io, file_name.c_str(), "ab");
	#else
		io = fopen(file_name.c_str(), "ab");
	#endif

	if(!io)
	{
		addonDebug("Cannot write/append to file %s", file_name.c_str());

		pushError.io = 1;
		pushError.clientid = clientid;
		pushError.file = file_name;
		pushError.error.assign("Cannot write/append to file");
		pushError.errorCode = 1;

		boost::mutex::scoped_lock lock(rMutex);
		amxFileErrorQueue.push(pushError);

		cPool = gPool->clientPool.find(clientid)->second;

		cPool.Transfer.Active = false;

		gPool->clientPool[clientid] = cPool;
		lock.unlock();

		return;
	}

	addonDebug("File %s opened in append/binary mode", file_name.c_str());

	socketid->read_some(boost::asio::buffer(buffer, sizeof buffer), error);

	if(error)
	{
		addonDebug("Error while receiving strat header from %i", clientid);

		pushError.io = 1;
		pushError.clientid = clientid;
		pushError.file = file_name;
		pushError.error.assign("Error while receiving start header");
		pushError.errorCode = 2;

		boost::mutex::scoped_lock lock(rMutex);
		amxFileErrorQueue.push(pushError);

		cPool = gPool->clientPool.find(clientid)->second;

		cPool.Transfer.Active = false;

		gPool->clientPool[clientid] = cPool;
		lock.unlock();

		fclose(io);

		return;
	}

	if(strcmp(buffer, "TCPQUERY CLIENT_CALL 2001 START"))
	{
		addonDebug("Invalid start header '%s' sent from clientid %i", buffer, clientid);

		pushError.io = 1;
		pushError.clientid = clientid;
		pushError.file = file_name;
		pushError.error.assign("Invalid start header sent");
		pushError.errorCode = 3;

		boost::mutex::scoped_lock lock(rMutex);
		amxFileErrorQueue.push(pushError);

		cPool = gPool->clientPool.find(clientid)->second;

		cPool.Transfer.Active = false;

		gPool->clientPool[clientid] = cPool;
		lock.unlock();

		fclose(io);

		return;
	}

	addonDebug("Received start header from %i", clientid);

	boost::this_thread::sleep(boost::posix_time::seconds(1));

	socketid->write_some(boost::asio::buffer("TCPQUERY SERVER_CALL 2001 READY", 32));

	addonDebug("Starting file receiving from %i...", clientid);

	while(total < file_length)
	{
		length = socketid->read_some(boost::asio::buffer(buffer, sizeof buffer), error);

		if(error)
		{
			addonDebug("Error while transfering file %s from client %i", file_name.c_str(), clientid);

			pushError.io = 1;
			pushError.clientid = clientid;
			pushError.file = file_name;
			pushError.error.assign("Error while transfering file");
			pushError.errorCode = 4;

			boost::mutex::scoped_lock lock(rMutex);
			amxFileErrorQueue.push(pushError);

			cPool = gPool->clientPool.find(clientid)->second;

			cPool.Transfer.Active = false;

			gPool->clientPool[clientid] = cPool;
			lock.unlock();

			fclose(io);

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

	/*recv(socketid, buffer, sizeof buffer, NULL);

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
	}*/

	socketid->write_some(boost::asio::buffer("TCPQUERY SERVER_CALL 2001 END", 30));

	addonDebug("File %s was successfuly received from %i", file_name.c_str(), clientid);

	amxFileReceive pushme;

	pushme.clientid = clientid;
	pushme.file = file_name;

	boost::mutex::scoped_lock lock(rMutex);
	amxFileReceiveQueue.push(pushme);

	cPool = gPool->clientPool.find(clientid)->second;

	cPool.Transfer.Active = false;

	gPool->clientPool[clientid] = cPool;
	lock.unlock();
}