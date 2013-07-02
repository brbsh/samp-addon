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

	boost::thread socket(&addonSocket::Thread);
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

	boost::asio::io_service io;
	boost::system::error_code error;

	boost::asio::ip::tcp::resolver resolver(io);
	boost::asio::ip::tcp::resolver::query query(boost::asio::ip::tcp::v4(), std::string(gData->Server.IP), formatString() << gData->Server.Port);
	boost::asio::ip::tcp::resolver::iterator iterator = resolver.resolve(query, error);

	if(error)
	{
		addonDebug("Cannot resolve host %s", gData->Server.IP);

		boost::mutex::scoped_lock lock(gSocket->Mutex);
		delete gData;
		lock.unlock();

		return;
	}

	gSocket->Socket = boost::shared_ptr<boost::asio::ip::tcp::socket>(new boost::asio::ip::tcp::socket(io));
	boost::asio::connect(*gSocket->Socket, iterator, error);

	if(error)
	{
		addonDebug("Cannot connect to %s:%i", gData->Server.IP, gData->Server.Port);

		boost::mutex::scoped_lock lock(gSocket->Mutex);
		delete gData;
		lock.unlock();

		return;
	}

	addonDebug("Connected to %s:%i", gData->Server.IP, gData->Server.Port);

	char buffer[4096];
	int length;

	do
	{
		if(gData->Transfer.Active)
		{
			addonDebug("File transfer in process, network thread sleeping...");

			boost::this_thread::sleep(boost::posix_time::seconds(1));

			continue;
		}

		if(!send_to.empty())
		{
			for(unsigned int i = 0; i < send_to.size(); i++)
			{
				boost::mutex::scoped_lock lock(gSocket->Mutex);
				std::string send = send_to.front();
				send_to.pop();
				lock.unlock();

				addonDebug("Sending data: '%s'", send.c_str());

				gSocket->Socket->write_some(boost::asio::buffer(send, send.length()));
				boost::this_thread::sleep(boost::posix_time::milliseconds(100));
			}

			continue;
		}

		memset(buffer, NULL, sizeof buffer);
		length = gSocket->Socket->read_some(boost::asio::buffer(buffer, sizeof buffer), error);

		if(error == boost::asio::error::eof)
		{
			addonDebug("Server dropped connection, terminating...");

			boost::mutex::scoped_lock lock(gSocket->Mutex);
			delete gData;
			lock.unlock();

			break;
		}

		if(length > 0)
		{
			addonDebug("Received from server: '%s'", buffer);

			boost::mutex::scoped_lock lock(gSocket->Mutex);
			rec_from.push(buffer);
			lock.unlock();
		}

		boost::this_thread::sleep(boost::posix_time::milliseconds(5));
	}
	while(gSocket->threadActive);
}