#pragma once



#include "tcpsocket.h"





addonSocket *gSocket;

std::queue<std::string> send_to;
std::queue<std::string> rec_from;


//extern addonMutex *gMutex;
//extern addonThread *gThread;





addonSocket::addonSocket()
{
	addonDebug("WSA constructor called");

	WSADATA wsaData;

	this->threadActive = false;
	this->Transfer.is = false;
	this->socketHandle = -1;

	WSAStartup(MAKEWORD(2, 2), &wsaData);

	if(LOBYTE(wsaData.wVersion) != 2 || HIBYTE(wsaData.wVersion) != 2) 
	{
		addonDebug("Error while initializing WSA (%i)", WSAGetLastError());

		this->Close();

		return;
	}

	addonDebug("WSA library was successfuly initialized");

	this->socketHandle = this->Create();
}



addonSocket::~addonSocket()
{
	addonDebug("WSA deconstructor called");

	if(this->threadActive)
		this->threadActive = false;

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

		return NULL;
	}

	addonDebug("Created TCP socket with id %i", socketH);

	return socketH;
}



void addonSocket::Close()
{
	if(this->threadActive)
	{
		send(this->socketHandle, "TCPQUERY CLIENT_CALL 1001 1", 27, NULL);

		shutdown(this->socketHandle, SD_BOTH);
		closesocket(this->socketHandle);
	}

	this->~addonSocket();
}



int addonSocket::Nonblock()
{
	if(this->socketHandle == -1)
		return NULL;
	
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

	this->threadActive = true;
	//this->Nonblock();

	boost::thread send(boost::bind(&addonSocket::SendThread, this->socketHandle));
	boost::thread receive(boost::bind(&addonSocket::ReceiveThread, this->socketHandle));
}




void addonSocket::Send(std::string data)
{
	if(!this->threadActive)
		return;

	//data.push_back('\n');

	boost::mutex::scoped_lock lock(this->Mutex);
	send_to.push(data);
	lock.unlock();
}



int addonSocket::GetInstance()
{
	return this->socketHandle;
}



void addonSocket::SendThread(int socketid)
{
	addonDebug("Thread addonSocket::SendThread(%i) succesfuly started", socketid);

	do
	{
		if(gSocket->Transfer.is)
		{
			boost::this_thread::sleep(boost::posix_time::milliseconds(100));

			continue;
		}

		if(!send_to.empty())
		{
			std::string getme;

			for(unsigned int i = 0; i < send_to.size(); i++)
			{
				boost::mutex::scoped_lock lock(gSocket->Mutex);
				getme = send_to.front();
				send_to.pop();
				lock.unlock();

				addonDebug("Sending data %s", getme.c_str());

				send(socketid, getme.c_str(), getme.length(), NULL);

				boost::this_thread::sleep(boost::posix_time::milliseconds(100));
			}
		}

		boost::this_thread::sleep(boost::posix_time::milliseconds(10));
	}
	while(gSocket->threadActive);
}



void addonSocket::ReceiveThread(int socketid)
{
	int bytes;
	char buffer[65536];
	
	addonDebug("Thread addonSocket::ReceiveThread(%i) successfuly started", socketid);

	do
	{
		if(gSocket->Transfer.is)
		{
			boost::this_thread::sleep(boost::posix_time::milliseconds(100));

			continue;
		}

		bytes = recv(socketid, (char *)&buffer, sizeof buffer, NULL);

		if(bytes > 0)
		{
			addonDebug("Recieved data: %s", buffer);

			boost::mutex::scoped_lock lock(gSocket->Mutex);
			rec_from.push(std::string(buffer));
			lock.unlock();

			memset(buffer, NULL, sizeof buffer);
		}

		boost::this_thread::sleep(boost::posix_time::milliseconds(10));
	}
	while(gSocket->threadActive);
}
