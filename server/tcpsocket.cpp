#pragma once



#include "tcpsocket.h"



extern amxMutex *gMutex;
extern amxPool *gPool;
extern amxThread *gThread;

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
	gThread->Stop(this->connHandle);
	gThread->Stop(this->recvHandle);
	gThread->Stop(this->sendHandle);

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
	for(std::map<int, sockPool>::iterator i = gPool->socketPool.begin(); i != gPool->socketPool.end(); i++) 
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

	this->connHandle = gThread->Start(amxSocket::ConnectionThread, (void *)this->socketID);
	this->recvHandle = gThread->Start(amxSocket::ReceiveThread, (void *)this->socketID);
	this->sendHandle = gThread->Start(amxSocket::SendThread, (void *)this->socketID);
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

		gMutex->Lock();
		amxDisconnectQueue.push(pushme);
		gPool->socketPool.erase(clientid);
		gPool->clientPool.erase(clientid);
		gMutex->unLock();

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

	gMutex->Lock();
	ret = gPool->socketPool[clientid].ip;
	gMutex->unLock();

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

		gMutex->Lock();
		sendQueue.push(pushme);
		gMutex->unLock();
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



#ifdef WIN32
	DWORD amxSocket::ConnectionThread(void *lpParam)
#else
	void *amxSocket::ConnectionThread(void *lpParam)
#endif
{
	int socketid = (int)lpParam;
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

			gMutex->Lock();
			gPool->socketPool[clientid] = pool;
			gMutex->unLock();

			pushme.ip = pool.ip;
			pushme.clientID = clientid;

			gMutex->Lock();
			amxConnectQueue.push(pushme);
			gMutex->unLock();
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

			gMutex->Lock();
			amxConnectErrorQueue.push(pushme);
			gMutex->unLock();
		}

		SLEEP(10);
	} 
	while((gSocket->socketInfo.active) && (gPool->pluginInit));

	#ifdef WIN32
		return true;
	#endif
}



#ifdef WIN32
	DWORD amxSocket::ReceiveThread(void *lpParam)
#else
	void *amxSocket::ReceiveThread(void *lpParam)
#endif
{
	int socketid = (int)lpParam;
	int sockID;
	int byte_len;
	char buffer[65536];

	memset(buffer, NULL, sizeof buffer);

	do 
	{
		for(std::map<int, sockPool>::iterator i = gPool->socketPool.begin(); i != gPool->socketPool.end(); i++) 
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

				gMutex->Lock();
				recvQueue.push(pushme);
				gMutex->unLock();
			}
			else if(byte_len == 0)
				gSocket->KickClient(i->first);
		}

		SLEEP(10);
	} 
	while((gSocket->socketInfo.active) && (gPool->pluginInit));

	#ifdef WIN32
		return true;
	#endif
}



#ifdef WIN32
	DWORD amxSocket::SendThread(void *lpParam)
#else
	void *amxSocket::SendThread(void *lpParam)
#endif
{
	processStruct getme;

	do
	{
		if(!sendQueue.empty())
		{
			for(unsigned int i = 0; i < sendQueue.size(); i++)
			{
				gMutex->Lock();
				getme = sendQueue.front();

				if(gPool->socketPool[getme.clientID].transfer)
				{
					gMutex->unLock();
					SLEEP(1);
					
					continue;
				}

				sendQueue.pop();
				gMutex->unLock();

				logprintf("Send data to %i: %s", getme.clientID, getme.data.c_str());

				send(gPool->socketPool[getme.clientID].socketid, getme.data.c_str(), getme.data.length(), NULL);
			}
		}

		SLEEP(10);
	}
	while((gSocket->socketInfo.active) && (gPool->pluginInit));

	#ifdef WIN32
		return true;
	#endif
}