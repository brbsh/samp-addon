#pragma once



#include "tcpsocket.h"



extern amxMutex *gMutex;
extern amxThread *gThread;

extern bool gInit;
extern logprintf_t logprintf;

extern std::queue<amxConnect> amxConnectQueue;
extern std::queue<amxConnectError> amxConnectErrorQueue;
extern std::queue<amxDisconnect> amxDisconnectQueue;

amxSocket *gSocket;
std::queue<sockStruct> sendQueue;
std::queue<sockStruct> recvQueue;





amxSocket::amxSocket()
{
	this->socketID = -1;
	this->maxClients = -1;

	#ifdef WIN32
		WORD wVersionRequested = MAKEWORD(2, 2);
		WSADATA wsaData;

		int err = WSAStartup(wVersionRequested, &wsaData);

		if(err || LOBYTE(wsaData.wVersion) != 2 || HIBYTE(wsaData.wVersion) != 2) 
		{
			logprintf("samp-addon: Winsock failed to initialize (error code: %i)", WSAGetLastError());

			this->Close();
		}
	#endif
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
		logprintf("samp-addon: socket_create() failed (function 'socket' returned -1)");

		return;
	}
}



void amxSocket::Close()
{
	for(int i = 0; i < this->maxClients; i++) 
	{
		if(this->clientsPool[i] != -1) 
		{
			this->CloseSocket(this->clientsPool[i]);
		}
	}

	free(this->clientsPool);

	this->CloseSocket(this->socketID);

	this->~amxSocket();
}



void amxSocket::CloseSocket(int socketid)
{
	#ifdef WIN32
		closesocket(socketid);
	#else
		close(socketid);
	#endif
}



void amxSocket::MaxClients(int max)
{
	if(this->socketID == -1)
		return;

	this->clientsPool = (int *)malloc(sizeof(int) * max);
	std::fill(this->clientsPool, (this->clientsPool + max), -1);
	this->maxClients = max;
}



int amxSocket::FindFreeSlot(int *pool, int size)
{
	for(int i = 0; i < size; i++)
	{
		if(pool[i] == -1)
			return i;
	}

	return 65535;
}



void amxSocket::Bind(std::string ip)
{
	if(this->socketID == -1)
		return;

	this->bind_ip.assign(ip);
}



void amxSocket::Listen(int port)
{
	if(this->socketID == -1)
		return;

	struct sockaddr_in addr;

	addr.sin_family = AF_INET;
	addr.sin_port = htons(port);

	memset(&(addr.sin_zero), 0, 8);

	
	if(!this->bind_ip.length()) 
	{
		addr.sin_addr.s_addr = htonl(INADDR_ANY);
	} 
	else 
	{
		addr.sin_addr.s_addr = inet_addr(this->bind_ip.c_str());
	}

	if(bind(this->socketID, (struct sockaddr *)&addr, sizeof(addr)) == -1) 
	{
		logprintf("[Socket]: socket_listen() - socket has failed to bind on port %i", port);

		return;
	}

	if(listen(this->socketID, 10) == -1) 
	{
		logprintf("[Socket]: socket_listen() - socket has failed to listen on port %i", port);

		return;
	}

	this->active = true;

	this->connHandle = gThread->Start(socket_connection_thread, (void *)this->socketID);
	this->recvHandle = gThread->Start(socket_receive_thread, (void *)this->socketID);
	this->sendHandle = gThread->Start(socket_send_thread, (void *)this->socketID);
}



void amxSocket::KickClient(int clientid)
{
	if(this->socketID == -1)
		return;

	int *client = &this->clientsPool[clientid];

	if(*client != -1)
	{
		this->CloseSocket(*client);

		*client = -1;
	}
}



bool amxSocket::IsClientConnected(int clientid)
{
	if(this->socketID == -1)
		return false;

	if((clientid <= this->maxClients) && (this->clientsPool[clientid] != -1))
	{
		char buffer[1024];

		if(recv(this->clientsPool[clientid], buffer, sizeof buffer, NULL) != 0)
			return true;
	}

	return false;
}



std::string amxSocket::GetClientIP(int clientid)
{
	if(this->socketID == -1)
		return std::string("");

	struct sockaddr_in peer_addr;

	#ifdef WIN32
		int cLen = sizeof(peer_addr);
	#else
		std::size_t cLen = sizeof(peer_addr);
	#endif

	getpeername(this->clientsPool[clientid], (struct sockaddr *)&peer_addr, &cLen);

	return std::string(inet_ntoa(peer_addr.sin_addr));
}



void amxSocket::Send(int clientid, std::string data)
{
	if(this->socketID == -1)
		return;

	if(this->clientsPool[clientid] != -1)
	{
		sockStruct pushme;

		pushme.clientid = clientid;
		pushme.data = data;

		gMutex->Lock();
		sendQueue.push(pushme);
		gMutex->unLock();
	}
}



int amxSocket::SetNonblock(int sockid)
{
    DWORD flags;

	#ifdef WIN32
		flags = 1;

		return ioctlsocket(sockid, FIONBIO, &flags);
	#else
		/* Fixme: O_NONBLOCK is defined but broken on SunOS 4.1.x and AIX 3.2.5. */
		if((flags = fcntl(sockid, F_GETFL, 0)) == -1)
        {
			flags = 0;
		}

		return fcntl(sockid, F_SETFL, (flags | O_NONBLOCK));
		fcntl(sockid, F_SETFL, O_NONBLOCK);
	#endif
}



#ifdef WIN32
	DWORD socket_connection_thread(void *lpParam)
#else
	void *socket_connection_thread(void *lpParam)
#endif
{
	int socketID = (int)lpParam;
	sockaddr_in remote_client;

	#ifdef WIN32
		int cLen = sizeof(remote_client);
	#else
		size_t cLen = sizeof(remote_client);
	#endif

	do
	{
		int client = accept(socketID, (sockaddr *)&remote_client, &cLen);
		int slot = gSocket->FindFreeSlot(gSocket->clientsPool, gSocket->maxClients);

		if((client != NULL) && (client != SOCKET_ERROR) && (slot != 65535)) 
		{
			amxConnect pushme;

			gSocket->SetNonblock(client);
			gSocket->clientsPool[slot] = client;

			pushme.ip.assign(inet_ntoa(remote_client.sin_addr));
			pushme.clientID = slot;

			gMutex->Lock();
			amxConnectQueue.push(pushme);
			gMutex->unLock();
		} 
		else 
		{
			amxConnectError pushme;

			pushme.ip.assign(inet_ntoa(remote_client.sin_addr));

			if(slot == 65535)
			{
				send(client, "-- Server is full", 17, 0);

				pushme.error.assign("Server is full");
				pushme.errorCode = 1;

				gMutex->Lock();
				amxConnectErrorQueue.push(pushme);
				gMutex->unLock();
			}

			gSocket->CloseSocket(client);

			if(pushme.errorCode != 1)
			{
				pushme.error.assign("Unknown error");

				gMutex->Lock();
				amxConnectErrorQueue.push(pushme);
				gMutex->unLock();
			}
		}

		SLEEP(1);
	} 
	while((gSocket->active) && (gInit));

	#ifdef WIN32
		return true;
	#endif
}



#ifdef WIN32
	DWORD socket_receive_thread(void *lpParam)
#else
	void *socket_receive_thread(void *lpParam)
#endif
{
	int max = gSocket->maxClients;
	int current;
	char szRecBuffer[32768];

	memset(szRecBuffer, NULL, sizeof szRecBuffer);

	do 
	{
		for(int i = 0; i < max; i++) 
		{
			current = gSocket->clientsPool[i];

			if(current != -1) 
			{
				int byte_len = recv(current, (char *)&szRecBuffer, sizeof szRecBuffer, NULL);

				if(byte_len > 0) 
				{
					sockStruct data;

					szRecBuffer[byte_len] = '\0';

					data.clientid = i;
					data.data.assign(szRecBuffer);

					memset(szRecBuffer, NULL, sizeof szRecBuffer);

					logprintf("Recieved data: %s", data.data.c_str());

					gMutex->Lock();
					recvQueue.push(data);
					gMutex->unLock();
				}

				if(!byte_len) 
				{
					amxDisconnect data;

					data.clientID = i;

					gSocket->CloseSocket(current);
					gSocket->clientsPool[i] = -1;

					gMutex->Lock();
					amxDisconnectQueue.push(data);
					gMutex->unLock();
				}
			}
		}

		SLEEP(1);
	} 
	while((gSocket->active) && (gInit));

	#ifdef WIN32
		return true;
	#endif
}



#ifdef WIN32
	DWORD socket_send_thread(void *lpParam)
#else
	void *socket_send_thread(void *lpParam)
#endif
{
	sockStruct data;

	do
	{
		if(!sendQueue.empty())
		{
			for(unsigned int i = 0; i < sendQueue.size(); i++)
			{
				gMutex->Lock();
				data = sendQueue.front();
				sendQueue.pop();
				gMutex->unLock();

				if(send(data.clientid, data.data.c_str(), data.data.length(), NULL) == SOCKET_ERROR)
				{
					logprintf("Error while sending data %s", data.data.c_str());

					continue;
				}

				SLEEP(25);
			}
		}

		SLEEP(1);
	}
	while((gSocket->active) && (gInit));

	#ifdef WIN32
		return true;
	#endif
}