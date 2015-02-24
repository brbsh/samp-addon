#pragma once



#include "client.hpp"





boost::asio::io_service gIOService;
boost::shared_ptr<addonSocket> gSocket;


extern boost::shared_ptr<addonCore> gCore;
extern boost::shared_ptr<addonD3Device> gD3Device;
extern boost::shared_ptr<addonDebug> gDebug;
extern boost::shared_ptr<addonPool> gPool;
extern boost::shared_ptr<addonTransfer> gTransfer;





addonSocket::addonSocket() : socket(io_s), connected(false)
{
	gDebug->traceLastFunction("addonSocket::addonSocket() at 0x?????");
	gDebug->Log("Called socket constructor");

	try
	{
		io_s.run();
	}
	catch(boost::system::system_error &err)
	{
		gDebug->Log("Cannot run I/O service (What: %s)", err.what());
	}

	connectionThreadInstance = boost::shared_ptr<boost::thread>(new boost::thread(boost::bind(&addonSocket::connectionThread, this)));
}



addonSocket::~addonSocket()
{
	gDebug->traceLastFunction("addonSocket::~addonSocket() at 0x?????");
	gDebug->Log("Called socket destructor");

	connectionThreadInstance->interrupt();
	recvThreadInstance->interrupt();
	socket.close();

	io_s.stop();
}



bool addonSocket::connectTo(std::string host, unsigned short port)
{
	assert(connectionThreadInstance->get_id() == boost::this_thread::get_id());

	gDebug->traceLastFunction("addonSocket::Connect(host = '%s', port = 0x%x) at 0x%x", host.c_str(), port, &addonSocket::connectTo);
	gDebug->Log("Attempting to connect to %s:%i trough TCP", host.c_str(), port);

	//gD3Device->clearRender();
	//gD3Device->renderText(strFormat() << "Connecting to " << host << ":" << port << "...", 10, 10, 1, 1, 1, 127);

	try
	{
		socket.connect(boost::asio::ip::tcp::endpoint(boost::asio::ip::address::from_string(host), port));

		char buffer[9];
		socket.read_some(boost::asio::buffer(buffer));
		buffer[8] = '\0';

		if(!strcmp(buffer, "ACCEPTED"))
		{
			gDebug->Log("Connect: Received 'ACCEPTED' flag from server");
			// connection accepted
		}
		else
		{
			socket.close();
			gDebug->Log("Cannot connect to %s:%i trough TCP (What: Connection rejected by server)", host.c_str(), port);

			return false;
		}
	}
	catch(boost::system::system_error &error)
	{
		//gD3Device->stopLastRender();
		//gD3Device->renderText(strFormat() << "Connecting to " << host << ":" << port << "... ERROR! " << error.what(), 10, 10, 255, 1, 1, 127);
		socket.close();
		gDebug->Log("Cannot connect to %s:%i trough TCP (What: %s)", host.c_str(), port, error.what());

		return false;
	}

	connected = true;
	//gD3Device->stopLastRender();
	//gD3Device->renderText(strFormat() << "Connecting to " << host << ":" << port << "... OK!", 10, 10, 1, 255, 1, 127);
	gDebug->Log("Connected to %s:%i trough TCP", host.c_str(), port);

	gTransfer = boost::shared_ptr<addonTransfer>(new addonTransfer(socket));

	writeTo(boost::str(boost::format("%1%|%2%|%3%|%4%|%5%") % gPool->getVar("serial") % 1488 % addonHash::crc32(host, host.length()) % port % gPool->getVar("name")));
	recvThreadInstance = boost::shared_ptr<boost::thread>(new boost::thread(boost::bind(&addonSocket::recvThread, this)));

	return true;
}



void addonSocket::disconnect()
{
	connected = false;
	recvThreadInstance->interrupt();
	recvThreadInstance.reset();
	gTransfer.reset();
	socket.close();
}



void addonSocket::writeTo(std::string data)
{
	while(gTransfer->isTransfering())
		boost::this_thread::sleep(boost::posix_time::seconds(1));

	data.insert(0, boost::str(boost::format("%1%|") % addonHash::crc32(data, data.length())));
	gDebug->Log("Sending '%s' (length: %i) to server", data.c_str(), data.length());

	try
	{
		socket.send(boost::asio::buffer(data, data.length()));
	}
	catch(boost::system::system_error& error)
	{
		gDebug->Log("Cannot sent data to server (%s)", error.what());

		disconnect();
	}
}



void addonSocket::connectionThread(addonSocket *instance)
{
	gDebug->traceLastFunction("addonSocket::connectionThread() at 0x%x", &addonSocket::connectionThread);
	gDebug->Log("Started socket connection thread");

	while(!GTA_PED_STATUS_ADDR) // PED context load flag
		boost::this_thread::sleep(boost::posix_time::seconds(1));

	while(true)
	{
		if(!instance->isConnected())
		{
			std::string ip = gPool->getVar("server_ip");
			unsigned short port = (atoi(gPool->getVar("server_port").c_str()) + 1);

			instance->connectTo(ip, port);
		}

		boost::this_thread::sleep(boost::posix_time::seconds(3));
	}
}



void addonSocket::recvThread(addonSocket *instance)
{
	assert(gSocket->getThreadInstance()->get_id() == boost::this_thread::get_id());

	gDebug->traceLastFunction("addonSocket::recvThread() at 0x%x", &addonSocket::recvThread);
	gDebug->Log("Started socket receive thread");

	while(true)
	{
		boost::this_thread::disable_interruption di;

		if(!gTransfer->isTransfering())
			gCore->queueIN(instance);

		boost::this_thread::restore_interruption re(di);
		boost::this_thread::sleep(boost::posix_time::milliseconds(100));
	}
}