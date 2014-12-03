#pragma once



#include "tcpsocket.h"





boost::asio::io_service gIOService;
boost::shared_ptr<addonSocket> gSocket;


extern boost::shared_ptr<addonCore> gCore;
extern boost::shared_ptr<addonDebug> gDebug;





addonSocket::addonSocket()
{
	gDebug->traceLastFunction("addonSocket::addonSocket() at 0x?????");
	gDebug->Log("Called socket constructor");

	boost::system::error_code error;

	gIOService.run(error);

	if(error)
		gDebug->Log("Cannot run I/O service: %s (Error code: %i)", error.message().c_str(), error.value());

	this->mutexSendInstance = boost::shared_ptr<boost::mutex>(new boost::mutex());
	this->mutexRecvInstance = boost::shared_ptr<boost::mutex>(new boost::mutex());
}



addonSocket::~addonSocket()
{
	gDebug->traceLastFunction("addonSocket::~addonSocket() at 0x?????");
	gDebug->Log("Called socket destructor");

	this->sendThreadInstance->interruption_requested();
	this->recvThreadInstance->interruption_requested();

	this->mutexSendInstance->destroy();
	this->mutexRecvInstance->destroy();

	this->socket->shutdown(boost::asio::socket_base::shutdown_both);
	this->socket->close();

	gIOService.stop();
}



bool addonSocket::Connect(std::string host, UINT port)
{
	assert(gCore->getThreadInstance()->get_id() == boost::this_thread::get_id());

	gDebug->traceLastFunction("addonSocket::Connect(host = '%s', port = 0x%x) at 0x%x", host.c_str(), port, &addonSocket::Connect);
	gDebug->Log("Attempting to connect to %s:%i trough TCP", host.c_str(), port);
	
	boost::system::error_code error;

	this->socket = boost::shared_ptr<boost::asio::ip::tcp::socket>(new boost::asio::ip::tcp::socket(gIOService));
	this->socket->connect(boost::asio::ip::tcp::endpoint(boost::asio::ip::address::from_string(host), port), error);

	if(error)
	{
		gDebug->Log("Cannot connect to %s:%i trough TCP: %s (Error code: %i)", host.c_str(), port, error.message().c_str(), error.value());

		return false;
	}
	else
	{
		gDebug->Log("Connected to %s:%i trough TCP", host.c_str(), port);
	}

	this->sendThreadInstance = boost::shared_ptr<boost::thread>(new boost::thread(boost::bind(&addonSocket::recvThread)));
	this->recvThreadInstance = boost::shared_ptr<boost::thread>(new boost::thread(boost::bind(&addonSocket::sendThread)));

	return true;
}



void addonSocket::sendThread()
{
	assert(gSocket->getThreadInstance(false)->get_id() == boost::this_thread::get_id());

	gDebug->traceLastFunction("addonSocket::sendThread() at 0x%x", &addonSocket::sendThread);
	gDebug->Log("Started socket send thread with id 0x%x", gSocket->getThreadInstance(false)->get_thread_info()->id);

	boost::system::error_code error;
	boost::asio::streambuf buffer;

	std::size_t length;

	while(true)
	{
		while(!gCore->outputQueue.empty())
		{
			boost::this_thread::disable_interruption di;

			std::ostream request(&buffer);
			std::pair<UINT, std::string> sendData;

			gSocket->getMutexInstance(false)->lock();
			sendData = gCore->outputQueue.front();
			gCore->outputQueue.pop();
			gSocket->getMutexInstance(false)->unlock();

			request << "TCPQUERY CLIENT_CALL" << " " << sendData.first << std::endl;
			request << "DATA" << " " << sendData.second << "\n.\n.\n.";

			length = boost::asio::write(*gSocket->getSocket(), buffer, error);

			if(error)
				gDebug->Log("Cannot write to socket: %s (Error code: %i)", error.message().c_str(), error.value());

			if(length < 1)
				gDebug->Log("NULL or negative length returned by socket write function");

			boost::this_thread::restore_interruption re(di);
			boost::this_thread::sleep_for(boost::chrono::milliseconds(1));
		}

		boost::this_thread::sleep_for(boost::chrono::milliseconds(50));
	}
}



void addonSocket::recvThread()
{
	assert(gSocket->getThreadInstance(true)->get_id() == boost::this_thread::get_id());

	gDebug->traceLastFunction("addonSocket::recvThread() at 0x%x", &addonSocket::recvThread);
	gDebug->Log("Started socket receive thread with id 0x%x", gSocket->getThreadInstance(true)->get_thread_info()->id);

	boost::system::error_code error;
	boost::asio::streambuf buffer;

	std::size_t length;

	while(true)
	{
		boost::this_thread::disable_interruption di;

		length = boost::asio::read_until(*gSocket->getSocket(), buffer, "\n.\n.\n.", error);

		if(error)
			gDebug->Log("Cannot read from socket: %s (Error code: %i)", error.message().c_str(), error.value());

		if(length > 0)
		{
			std::istream response(&buffer);
			std::pair<UINT, std::string> pushData;

			response >> pushData.first;
			std::getline(response, pushData.second);

			gSocket->getMutexInstance(true)->lock();
			gCore->pendingQueue.push(pushData);
			gSocket->getMutexInstance(true)->unlock();
		}

		boost::this_thread::restore_interruption re(di);
		boost::this_thread::sleep_for(boost::chrono::milliseconds(50));
	}
}