#pragma once

#include "tcpsocket.h"



extern addonMutex *gMutex;
extern addonThread *gThread;
extern addonSocket *gSocket;


std::queue<std::string> send_to;
std::queue<std::string> rec_from;





addonSocket::addonSocket()
{
	addonDebug("WSA constructor called");

	WORD wVersionRequested;
	WSADATA wsaData;
	wVersionRequested = MAKEWORD(2, 2);
	WSAStartup(wVersionRequested, &wsaData);

	this->active = false;
	this->socketHandle = -1;
	
	if(LOBYTE(wsaData.wVersion) != 2 || HIBYTE(wsaData.wVersion) != 2) 
	{
		addonDebug("Error while initializing WSA (%i)", WSAGetLastError());

		this->Close();

		return;
	}

	addonDebug("WSA library was successfuly initialized");
}



addonSocket::~addonSocket()
{
	addonDebug("WSA deconstructor called");

	if(this->active)
	{
		this->active = false;

		gThread->Stop(this->sendHandle);
		gThread->Stop(this->receiveHandle);
	}

	this->socketHandle = -1;

	WSACleanup();

	addonDebug("WSA library was successfuly cleaned up");
}



int addonSocket::Create()
{
	addonDebug("Trying to create TCP socket...");

	int socketH = socket(AF_INET, SOCK_STREAM, NULL);

	if(socketH == -1)
	{
		addonDebug("Error while creating new socket (%i)", WSAGetLastError());

		this->Close();
	}

	addonDebug("Created TCP socket with id %i", socketH);

	return socketH;
}



void addonSocket::Close()
{
	if(this->active)
		closesocket(this->socketHandle);

	this->~addonSocket();
}



int addonSocket::set_nonblocking_socket()
{
	if(this->socketHandle == -1)
		return 0;

    DWORD flags = 1;

	return ioctlsocket(this->socketHandle, FIONBIO, &flags);
} 



void addonSocket::Connect(std::string address, int port)
{
	if(this->socketHandle == -1)
		return;

	addonDebug("Connecting to %s:%i ...", address.c_str(), port);

	struct sockaddr_in addr;
	struct hostent *host;

	host = gethostbyname(address.c_str());

	addr.sin_family = AF_INET;
	addr.sin_port = htons(port);
	addr.sin_addr = *((struct in_addr *)host->h_addr);

	if(connect(this->socketHandle, (struct sockaddr *)&addr, sizeof(sockaddr)) == SOCKET_ERROR) 
	{
		addonDebug("Cannot connect to %s:%i trough TCP socket", address.c_str(), port);

		this->Close();

		return;
	}

	addonDebug("Connected to %s:%i trough TCP", address.c_str(), port);

	this->active = true;
	this->set_nonblocking_socket();

	this->sendHandle = gThread->Start((LPTHREAD_START_ROUTINE)socket_send_thread, NULL);
	this->receiveHandle = gThread->Start((LPTHREAD_START_ROUTINE)socket_receive_thread, NULL);
}




void addonSocket::Send(std::string data)
{
	if(!this->active)
		return;

	gMutex->Lock();
	send_to.push(data);
	gMutex->unLock();
}



DWORD __stdcall socket_send_thread(LPVOID lpParam)
{
	addonDebug("Thread 'socket_send_thread' succesfuly started");

	std::string data;

	while(gSocket->active)
	{
		if(!send_to.empty())
		{
			addonDebug("Send queue is not empty, processing...");

			for(unsigned int i = 0; i < send_to.size(); i++)
			{
				gMutex->Lock();
				data = send_to.front();
				send_to.pop();
				gMutex->unLock();

				data.push_back('\n');

				send(gSocket->socketHandle, data.c_str(), data.length(), NULL);
			}

			addonDebug("All items in send queue was processed");
		}

		Sleep(50);
	}

	return true;
}



DWORD __stdcall socket_receive_thread(LPVOID lpParam)
{
	addonDebug("Thread 'socket_receive_thread' successfuly started");

	int bytes;
	char buffer[32768];
	std::string data;

	while(gSocket->active)
	{
		bytes = recv(gSocket->socketHandle, buffer, sizeof buffer, NULL);

		if(bytes > 0)
		{
			addonDebug("There is data received, saving...");

			data.assign(buffer);

			addonDebug("New data: %s", data.c_str());

			gMutex->Lock();
			rec_from.push(data);
			gMutex->unLock();

			bytes = 0;
			memset(buffer, NULL, sizeof buffer);
		}

		Sleep(50);
	}

	return true;
}
