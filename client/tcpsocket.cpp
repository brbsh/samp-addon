#pragma once



#include "tcpsocket.hpp"





boost::asio::io_service gIOService;
boost::shared_ptr<addonSocket> gSocket;


extern boost::shared_ptr<addonCore> gCore;
extern boost::shared_ptr<addonDebug> gDebug;
extern boost::shared_ptr<addonPool> gPool;





addonSocket::addonSocket()
{
	gDebug->traceLastFunction("addonSocket::addonSocket() at 0x?????");
	gDebug->Log("Called socket constructor");

	try
	{
		this->io_s.run();
	}
	catch(boost::system::system_error &err)
	{
		gDebug->Log("Cannot run I/O service (What: %s)", err.what());
	}
}



addonSocket::~addonSocket()
{
	gDebug->traceLastFunction("addonSocket::~addonSocket() at 0x?????");
	gDebug->Log("Called socket destructor");

	this->sendThreadInstance->interruption_requested();
	this->recvThreadInstance->interruption_requested();

	this->socket->shutdown(boost::asio::socket_base::shutdown_both);
	this->socket->close();

	this->io_s.stop();
}



bool addonSocket::Connect(std::string host, UINT port)
{
	assert(gCore->getThreadInstance()->get_id() == boost::this_thread::get_id());

	gDebug->traceLastFunction("addonSocket::Connect(host = '%s', port = 0x%x) at 0x%x", host.c_str(), port, &addonSocket::Connect);
	gDebug->Log("Attempting to connect to %s:%i trough TCP", host.c_str(), port);
	
	boost::unique_lock<boost::shared_mutex> lockit(this->sockMutex);
	this->socket = boost::shared_ptr<boost::asio::ip::tcp::socket>(new boost::asio::ip::tcp::socket(this->io_s));
	lockit.unlock();

	try
	{
		lockit.lock();
		this->socket->connect(boost::asio::ip::tcp::endpoint(boost::asio::ip::address::from_string(host), port));
		lockit.unlock();
	}
	catch(boost::system::system_error &err)
	{
		gDebug->Log("Cannot connect to %s:%i trough TCP (What: %s)", host.c_str(), port, err.what());

		return false;
	}

	gDebug->Log("Connected to %s:%i trough TCP", host.c_str(), port);

	std::string initial;
	initial += "SAMPADDON";
	initial += gPool->getVar("playerSerial");
	initial += "j";
	gDebug->Log("Sending data %s", initial.c_str());

	this->socket->write_some(boost::asio::buffer(initial, initial.length()));

	this->sendThreadInstance = boost::shared_ptr<boost::thread>(new boost::thread(boost::bind(&addonSocket::recvThread, this->socket)));
	this->recvThreadInstance = boost::shared_ptr<boost::thread>(new boost::thread(boost::bind(&addonSocket::sendThread, this->socket)));

	return true;
}



void addonSocket::sendThread(boost::shared_ptr<boost::asio::ip::tcp::socket> socket)
{
	assert(gSocket->getThreadInstance(false)->get_id() == boost::this_thread::get_id());

	gDebug->traceLastFunction("addonSocket::sendThread() at 0x%x", &addonSocket::sendThread);
	//gDebug->Log("Started socket send thread with id 0x%x", gSocket->getThreadInstance(false)->get_thread_info()->id);

	while(true)
	{
		boost::this_thread::disable_interruption di;

		gCore->queueOUT(socket);

		boost::this_thread::restore_interruption re(di);
		boost::this_thread::sleep_for(boost::chrono::milliseconds(50));
	}
}



void addonSocket::recvThread(boost::shared_ptr<boost::asio::ip::tcp::socket> socket)
{
	assert(gSocket->getThreadInstance(true)->get_id() == boost::this_thread::get_id());

	gDebug->traceLastFunction("addonSocket::recvThread() at 0x%x", &addonSocket::recvThread);
	//gDebug->Log("Started socket receive thread with id 0x%x", gSocket->getThreadInstance(true)->get_thread_info()->id);

	while(true)
	{
		boost::this_thread::disable_interruption di;

		gCore->queueIN(socket);

		boost::this_thread::restore_interruption re(di);
		boost::this_thread::sleep_for(boost::chrono::milliseconds(50));
	}
}