#pragma once



#include "tcpsocket.h"





addonSocket *gSocket;

std::queue<std::string> send_to;
std::queue<std::string> rec_from;


extern addonData *gData;





addonSocket::addonSocket()
{
	addonDebug("Socket constructor called");

	gData->Server.Port++;

	boost::mutex::scoped_lock lock(this->Mutex);
	this->threadActive = true;
	lock.unlock();

	boost::thread socket(boost::bind(&addonSocket::Thread));
}



addonSocket::~addonSocket()
{
	addonDebug("Socket deconstructor called");

	boost::mutex::scoped_lock lock(this->Mutex);
	this->threadActive = false;
	lock.unlock();

	this->Socket->close();
}



void addonSocket::Send(std::string data)
{
	if(!this->threadActive)
		return;

	boost::mutex::scoped_lock lock(this->Mutex);
	send_to.push(data);
	lock.unlock();
}



void addonSocket::Thread()
{
	addonDebug("Thread addonSocket::Thread() successfuly started");
	addonDebug("Connecting to %s:%i", gData->Server.IP, gData->Server.Port);

	boost::mutex tMutex;
	boost::asio::io_service io;
	boost::system::error_code error;

	gSocket->Socket = boost::shared_ptr<boost::asio::ip::tcp::socket>(new boost::asio::ip::tcp::socket(io));
	gSocket->Socket->connect(boost::asio::ip::tcp::endpoint(boost::asio::ip::address::from_string(gData->Server.IP), gData->Server.Port), error);

	if(error)
	{
		addonDebug("Cannot connect to %s:%i", gData->Server.IP, gData->Server.Port);

		boost::mutex::scoped_lock lock(tMutex);
		delete gData;
		lock.unlock();

		return;
	}

	addonDebug("Connected to %s:%i", gData->Server.IP, gData->Server.Port);

	boost::thread send(boost::bind(&addonSocket::SendThread));
	boost::thread receive(boost::bind(&addonSocket::ReceiveThread));

	while(gSocket->threadActive)
	{
		boost::this_thread::sleep(boost::posix_time::seconds(1));
	}

	gSocket->Socket->close();
}



void addonSocket::SendThread()
{
	addonDebug("Thread addonSocket::SendThread() successfuly started");

	boost::mutex sMutex;
	std::string data;
	std::size_t length;

	do
	{
		if(!send_to.empty())
		{
			for(unsigned int i = 0; i < send_to.size(); i++)
			{
				boost::mutex::scoped_lock lock(sMutex);
				data = send_to.front();
				send_to.pop();
				lock.unlock();

				addonDebug("Sending to server: '%s'", data.c_str());

				length = gSocket->Socket->write_some(boost::asio::buffer(data));

				addonDebug("Sent %i bytes", length);

				boost::this_thread::sleep(boost::posix_time::milliseconds(1));
			}
		}

		boost::this_thread::sleep(boost::posix_time::milliseconds(1));
	}
	while(gSocket->threadActive);
}



void addonSocket::ReceiveThread()
{
	addonDebug("Thread addonSocket::ReceiveThread() successfuly started");

	char buffer[8192];

	boost::mutex rMutex;
	boost::system::error_code error;
	std::size_t length;

	do
	{
		memset(buffer, NULL, sizeof buffer);

		length = gSocket->Socket->read_some(boost::asio::buffer(buffer, sizeof buffer), error);

		if(error == boost::asio::error::eof)
		{
			addonDebug("Server dropped connection, terminating...");

			boost::mutex::scoped_lock lock(rMutex);
			delete gData;
			lock.unlock();

			break;
		}

		if(length > 0)
		{
			std::string data;

			addonDebug("Received %i bytes", length);

			data.assign(buffer, length);

			addonDebug("Received from server: '%s'", data.c_str());

			boost::mutex::scoped_lock lock(rMutex);
			rec_from.push(data);
			lock.unlock();
		}

		boost::this_thread::sleep(boost::posix_time::milliseconds(1));
	}
	while(gSocket->threadActive);
}