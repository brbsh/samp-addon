#pragma once



#include "tcpsocket.h"



//extern amxMutex *gMutex;
extern amxPool *gPool;
//extern amxThread *gThread;

extern logprintf_t logprintf;


extern std::queue<amxConnect> amxConnectQueue;
extern std::queue<amxConnectError> amxConnectErrorQueue;
extern std::queue<amxDisconnect> amxDisconnectQueue;


amxSocket *gSocket;

std::queue<processStruct> sendQueue;
std::queue<processStruct> recvQueue;





amxSocket::amxSocket()
{
	this->socketID = -1;
	this->socketInfo.maxClients = -1;

	#ifdef WIN32
		WSADATA wsaData;

		int err = WSAStartup(MAKEWORD(2, 2), &wsaData);

		if(err || LOBYTE(wsaData.wVersion) != 2 || HIBYTE(wsaData.wVersion) != 2) 
		{
			logprintf("samp-addon: Winsock failed to initialize (error code: %i)", WSAGetLastError());

			this->Close();
		}
	#endif

	this->Create();
}



amxSocket::~amxSocket()
{
	#ifdef WIN32
		WSACleanup();
	#endif
}



void amxSocket::Create()
{
	this->socketID = socket(AF_INET, SOCK_STREAM, NULL);

	if(this->socketID == -1)
	{
		logprintf("samp-addon: socket creation failed (function 'socket' returned -1)");

		return;
	}
}



void amxSocket::Close()
{
	for(boost::unordered_map<int, sockPool>::iterator i = gPool->socketPool.begin(); i != gPool->socketPool.end(); i++) 
	{
		if(i->second.socketid != -1) 
		{
			this->CloseSocket(i->second.socketid);
		}
	}

	this->CloseSocket(this->socketID);

	this->~amxSocket();
}



void amxSocket::CloseSocket(int socketid)
{
	#ifdef WIN32
		shutdown(socketid, SD_BOTH);
		closesocket(socketid);
	#else
		close(socketid);
	#endif
}



void amxSocket::MaxClients(int max)
{
	if(this->socketID == -1)
		return;

	this->socketInfo.maxClients = max;
}



int amxSocket::FindFreeSlot()
{
	for(unsigned int i = 0; i != this->socketInfo.maxClients; i++)
	{
		if(!gPool->socketPool.count(i))
			return i;
	}

	return 65535;
}



void amxSocket::Bind(std::string ip)
{
	if(this->socketID == -1)
		return;

	this->bind_ip = ip;
}



void amxSocket::Listen(int port)
{
	if(this->socketID == -1)
		return;

	struct sockaddr_in addr;

	addr.sin_family = AF_INET;
	addr.sin_port = htons(port);

	memset(&(addr.sin_zero), NULL, 8);

	if(!this->bind_ip.length()) 
		addr.sin_addr.s_addr = htonl(INADDR_ANY);
	else 
		addr.sin_addr.s_addr = inet_addr(this->bind_ip.c_str());

	if(bind(this->socketID, (struct sockaddr *)&addr, sizeof(addr)) == SOCKET_ERROR) 
	{
		logprintf("samp-addon: socket has failed to bind on port %i", port);

		return;
	}

	if(listen(this->socketID, SOMAXCONN) == SOCKET_ERROR) 
	{
		logprintf("samp-addon: socket has failed to listen on port %i", port);

		return;
	}

	logprintf("\nsamp-addon: started TCP server on port %i\n", port);

	this->socketInfo.active = true;

	boost::thread connection(boost::bind(&amxSocket::ConnectionThread, this->socketID));
	boost::thread receive(boost::bind(&amxSocket::ReceiveThread, this->socketID));
	boost::thread send(boost::bind(&amxSocket::SendThread, this->socketID));
}



void amxSocket::KickClient(int clientid)
{
	if(this->socketID == -1)
		return;

	int socketid = gPool->socketPool[clientid].socketid;

	if(socketid != -1)
	{
		amxDisconnect pushme;

		pushme.clientID = clientid;

		boost::mutex::scoped_lock lock(this->Mutex);
		amxDisconnectQueue.push(pushme);
		lock.unlock();

		boost::mutex::scoped_lock pool_lock(gPool->Mutex);
		gPool->socketPool.erase(clientid);
		gPool->clientPool.erase(clientid);
		pool_lock.unlock();

		this->CloseSocket(socketid);
	}
}



bool amxSocket::IsClientConnected(int clientid)
{
	if(this->socketID == -1)
		return false;

	int socketid = gPool->socketPool[clientid].socketid;

	if((clientid <= this->socketInfo.maxClients) && socketid && gPool->clientPool[clientid].auth)
	{
		char buffer[65536];

		if(recv(socketid, buffer, sizeof buffer, NULL) != 0)
			return true;
	}

	return false;
}



std::string amxSocket::GetClientIP(int clientid)
{
	if(!this->IsClientConnected(clientid))
		return std::string("0.0.0.0");

	std::string ret;

	boost::mutex::scoped_lock lock(gPool->Mutex);
	ret = gPool->socketPool[clientid].ip;
	lock.unlock();

	return ret;
}



void amxSocket::Send(int clientid, std::string data)
{
	if(this->socketID == -1)
		return;

	if(this->IsClientConnected(clientid))
	{
		processStruct pushme;

		pushme.clientID = clientid;
		pushme.data = data;

		boost::mutex::scoped_lock lock(this->Mutex);
		sendQueue.push(pushme);
		lock.unlock();
	}
}



int amxSocket::SetNonblock(int socketid)
{
	if(socketid == -1)
		return NULL;

    DWORD flags;

	#ifdef WIN32
		flags = 1;

		return ioctlsocket(socketid, FIONBIO, &flags);
	#else
		/* Fixme: O_NONBLOCK is defined but broken on SunOS 4.1.x and AIX 3.2.5. */
		if((flags = fcntl(socketid, F_GETFL, 0)) == -1)
        {
			flags = 0;
		}

		return fcntl(socketid, F_SETFL, (flags | O_NONBLOCK));
		fcntl(socketid, F_SETFL, O_NONBLOCK);
	#endif
}



void amxSocket::ConnectionThread(int socketid)
{
	sockaddr_in remote_client;

	#ifdef WIN32
		int cLen = sizeof(remote_client);
	#else
		size_t cLen = sizeof(remote_client);
	#endif

	do
	{
		int sockID = accept(socketid, (sockaddr *)&remote_client, &cLen);
		int clientid = gSocket->FindFreeSlot();

		if((sockID != SOCKET_ERROR) && (clientid != 65535)) 
		{
			amxConnect pushme;
			sockPool pool;

			//gSocket->SetNonblock(sockID);

			pool.ip.assign(inet_ntoa(remote_client.sin_addr));
			pool.socketid = sockID;
			pool.transfer = false;

			boost::mutex::scoped_lock pool_lock(gPool->Mutex);
			gPool->socketPool[clientid] = pool;
			pool_lock.unlock();

			pushme.ip = pool.ip;
			pushme.clientID = clientid;

			boost::mutex::scoped_lock lock(gSocket->Mutex);
			amxConnectQueue.push(pushme);
			lock.unlock();
		} 
		else 
		{
			amxConnectError pushme;

			pushme.ip.assign(inet_ntoa(remote_client.sin_addr));

			if(sockID == SOCKET_ERROR)
			{
				pushme.error.assign("Cannot accept new client (\"accept\" returned -1)");
				pushme.errorCode = NULL;
			}

			if(clientid == 65535)
			{
				send(sockID, "TCPQUERY SERVER_CALL 1001 1", 27, NULL);

				pushme.error.assign("Server is full");
				pushme.errorCode = 1;
			}

			gSocket->KickClient(sockID);

			boost::mutex::scoped_lock lock(gSocket->Mutex);
			amxConnectErrorQueue.push(pushme);
			lock.unlock();
		}

		boost::this_thread::sleep(boost::posix_time::milliseconds(1));
	} 
	while((gSocket->socketInfo.active) && (gPool->pluginInit));
}



void amxSocket::ReceiveThread(int socketid)
{
	int sockID;
	int byte_len;
	char buffer[65536];

	memset(buffer, NULL, sizeof buffer);

	do 
	{
		for(boost::unordered_map<int, sockPool>::iterator i = gPool->socketPool.begin(); i != gPool->socketPool.end(); i++) 
		{
			if(i->second.transfer)
				continue;

			sockID = i->second.socketid;
			byte_len = recv(sockID, (char *)&buffer, sizeof buffer, NULL);

			if(byte_len > 0) 
			{
				processStruct pushme;

				buffer[byte_len] = NULL;

				pushme.clientID = i->first;
				pushme.data.assign(buffer);

				memset(buffer, NULL, sizeof buffer);

				logprintf("Received data from %i: %s", i->first, pushme.data.c_str());

				boost::mutex::scoped_lock lock(gSocket->Mutex);
				recvQueue.push(pushme);
				lock.unlock();
			}
			else if(!byte_len)
				gSocket->KickClient(i->first);
		}

		boost::this_thread::sleep(boost::posix_time::milliseconds(1));
	} 
	while((gSocket->socketInfo.active) && (gPool->pluginInit));
}



void amxSocket::SendThread(int socketid)
{
	processStruct getme;

	do
	{
		if(!sendQueue.empty())
		{
			for(unsigned int i = 0; i < sendQueue.size(); i++)
			{
				boost::mutex::scoped_lock lock(gSocket->Mutex);
				getme = sendQueue.front();
				sendQueue.pop();
				lock.unlock();

				if(gPool->socketPool[getme.clientID].transfer)
					continue;

				logprintf("Send data to %i: %s", getme.clientID, getme.data.c_str());

				send(gPool->socketPool[getme.clientID].socketid, getme.data.c_str(), getme.data.length(), NULL);
			}
		}

		boost::this_thread::sleep(boost::posix_time::milliseconds(1));
	}
	while((gSocket->socketInfo.active) && (gPool->pluginInit));
}