#pragma once

#include "tcpsocket.h"

extern addonThread* gThread;
extern addonMutex* gMutex;
extern addonSocket* gSocket;


std::queue<std::string> send_to;
std::queue<std::string> rec_from;
extern std::queue<char> keyQueue;




addonSocket::addonSocket()
{
	addonDebug("WSA constructor called from 'main_thread'");

	WORD wVersionRequested;
	WSADATA wsaData;
	wVersionRequested = MAKEWORD(2, 2);
	WSAStartup(wVersionRequested, &wsaData);

	this->socket_active = false;
	this->receive_thread = NULL;
	
	if(LOBYTE(wsaData.wVersion) != 2 || HIBYTE(wsaData.wVersion) != 2) 
	{
		addonDebug("Error while initializing WSA (%i)", WSAGetLastError());

		this->Close();
	}

	addonDebug("WSA library was successfuly initialized");

	this->socket_handle = this->Start();
	this->Connect(std::string("127.0.0.1"), 7700);
}



addonSocket::~addonSocket()
{
	WSACleanup();

	addonDebug("WSA library was successfuly cleaned up");

	if(this->socket_active)
	{
		this->socket_active = false;

		gThread->Stop(this->receive_thread);
	}
}



int addonSocket::Start()
{
	int socket_handle = socket(AF_INET, SOCK_STREAM, NULL);

	if(socket_handle == -1)
	{
		addonDebug("Error while creating new socket (%i)", WSAGetLastError());

		this->Close();
	}

	return socket_handle;
}



void addonSocket::Close()
{
	closesocket(this->socket_handle);

	this->~addonSocket();
}



int addonSocket::set_nonblocking_socket(int fd)
{
    DWORD flags = 1;

	return ioctlsocket(fd, FIONBIO, &flags);
} 



void addonSocket::Connect(std::string address, int port)
{
	if(this->socket_handle == -1)
		return;

	struct sockaddr_in addr;
	struct hostent *host;

	host = gethostbyname(address.c_str());

	addr.sin_family = AF_INET;
	addr.sin_port = htons(port);
	addr.sin_addr = *((struct in_addr *)host->h_addr);

	if(connect(this->socket_handle, (struct sockaddr *)&addr, sizeof(sockaddr)) == SOCKET_ERROR) 
	{
		addonDebug("Cannot connect to %s:%i trough TCP socket", address.c_str(), port);

		this->Close();

		return;
	}

	//this->set_nonblocking_socket(this->socket_handle);
	this->socket_active = true;

	this->send_thread = gThread->Start((LPTHREAD_START_ROUTINE)socket_send_thread);
	this->receive_thread = gThread->Start((LPTHREAD_START_ROUTINE)socket_receive_thread);
}




void addonSocket::Send(std::string data)
{
	gMutex->Lock();
	send_to.push(data);
	gMutex->unLock();
}



DWORD _stdcall socket_send_thread(LPVOID lpParam)
{
	addonDebug("Thread 'socket_send_thread' succesfuly started");

	char key;
	std::string data;

	while(gSocket->socket_active)
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
				send(gSocket->socket_handle, data.c_str(), data.length(), NULL);
			}

			addonDebug("All items in send queue was processed");
		}

		if(!keyQueue.empty())
		{
			for(unsigned int i = 0; i < keyQueue.size(); i++)
			{
				gMutex->Lock();
				key = keyQueue.front();
				keyQueue.pop();
				gMutex->unLock();

				data.assign("TCPQUERY>CLIENT_CALL>KEY_PRESSED>");
				data.push_back(key);
				data.push_back('\n');
				send(gSocket->socket_handle, data.c_str(), data.length(), NULL);
			}
		}

		Sleep(50);
	}

	return true;
}



DWORD _stdcall socket_receive_thread(LPVOID lpParam)
{
	addonDebug("Thread 'socket_receive_thread' successfuly started");

	std::string data;
	char buffer[16384];
	int bytes;

	while(gSocket->socket_active)
	{
		bytes = recv(gSocket->socket_handle, buffer, sizeof buffer, NULL);

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
