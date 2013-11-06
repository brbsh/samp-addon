#pragma once



#include "tcpsocket.h"





boost::asio::io_service gIOService;
boost::shared_ptr<addonSocket> gSocket;


extern boost::shared_ptr<addonDebug> gDebug;





addonSocket::addonSocket()
{
	gDebug->traceLastFunction("addonSocket::addonSocket() at 0x?????");

	boost::system::error_code error;

	gIOService.run(error);

	if(error)
		gDebug->Log("Cannot run I/O service: %s (%i)", error.message().c_str(), boost::lexical_cast<int>(error));

	this->mutexInstance = boost::shared_ptr<boost::mutex>(new boost::mutex());
}



addonSocket::~addonSocket()
{
	gDebug->traceLastFunction("addonSocket::~addonSocket() at 0x?????");

	this->mutexInstance->destroy();
	this->sendThreadHandle->interrupt();
	this->recvThreadHandle->interrupt();
	this->socket->shutdown(boost::asio::socket_base::shutdown_both);
	this->socket->close();

	gIOService.stop();
}



void addonSocket::Connect(std::string host, unsigned short port)
{
	gDebug->traceLastFunction("addonSocket::Connect(host = '%s', port = 0x%x) at 0x%x", host.c_str(), port, &addonSocket::Connect);
	
	boost::system::error_code error;

	this->socket = boost::shared_ptr<boost::asio::ip::tcp::socket>(new boost::asio::ip::tcp::socket(this->IOService));
	boost::asio::ip::tcp::endpoint endpoint(boost::asio::ip::address_v4::from_string(host), port);
	this->socket->connect(endpoint, error);

	if(error)
		gDebug->Log("Cannot connect to %s:%i: %s (%i)", host.c_str(), port, error.message().c_str(), boost::lexical_cast<int>(error));
	else
		gDebug->Log("Connected to %s:%i trough TCP", host.c_str(), port);

	this->sendThreadHandle = boost::shared_ptr<boost::thread>(new boost::thread(boost::bind(&addonSocket::recvThread, this)));
	this->recvThreadHandle = boost::shared_ptr<boost::thread>(new boost::thread(boost::bind(&addonSocket::sendThread, this)));
}



void addonSocket::sendThread(addonSocket *sock)
{
	assert(gSocket->getThreadInstance(false)->get_id() == boost::this_thread::get_id());

	gDebug->traceLastFunction("addonSocket::sendThread(sock = 0x%x) at 0x%x", &sock, &addonSocket::sendThread);

	boost::mutex tMutex;

	while(true)
	{
		/*if(true) // if(!queue.empty())
		{
			boost::this_thread::disable_interruption di;

			boost::this_thread::sleep_for(boost::chrono::milliseconds(1));
		}*/

		boost::this_thread::sleep_for(boost::chrono::milliseconds(50));
	}
}



void addonSocket::recvThread(addonSocket *sock)
{
	assert(gSocket->getThreadInstance(true)->get_id() == boost::this_thread::get_id());

	gDebug->traceLastFunction("addonSocket::recvThread(sock = 0x%x) at 0x%x", &sock, &addonSocket::recvThread);

	boost::mutex tMutex;

	while(true)
	{
		/*if(true) // if(!queue.empty())
		{
			boost::this_thread::disable_interruption di;

			boost::this_thread::sleep_for(boost::chrono::milliseconds(1));
		}*/

		boost::this_thread::sleep_for(boost::chrono::milliseconds(50));
	}
}