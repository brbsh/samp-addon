#pragma once



#include "core.hpp"





boost::shared_ptr<addonCore> gCore;


extern boost::shared_ptr<addonD3Device> gD3Device;
extern boost::shared_ptr<addonDebug> gDebug;
extern boost::shared_ptr<addonFunctions> gFunctions;
extern boost::shared_ptr<addonLoader> gLoader;
extern boost::shared_ptr<addonPool> gPool;
extern boost::shared_ptr<addonSocket> gSocket;





addonCore::addonCore()
{
	gFunctions = boost::shared_ptr<addonFunctions>(new addonFunctions());
	gPool = boost::shared_ptr<addonPool>(new addonPool());
	gSocket = boost::shared_ptr<addonSocket>(new addonSocket());

	gDebug->traceLastFunction("addonCore::addonCore() at 0x?????");
	gDebug->Log("Core constructor called");

	threadInstance = boost::shared_ptr<boost::thread>(new boost::thread(boost::bind(&addonCore::Thread)));
}



addonCore::~addonCore()
{
	gDebug->traceLastFunction("addonCore::~addonCore() at 0x?????");
	gDebug->Log("Core destructor called");

	threadInstance->interruption_requested();
	threadInstance.reset();

	pqMutex.destroy();
	ouMutex.destroy();

	gLoader.reset();
	gSocket.reset();
	gPool.reset();
	gFunctions.reset();
}



void addonCore::queueIN(boost::shared_ptr<boost::asio::ip::tcp::socket> sock)
{
	boost::asio::streambuf buffer;
	std::size_t length;

	try
	{
		length = boost::asio::read_until(*sock.get(), buffer, "\n.\n.\n.");
	}
	catch(boost::system::system_error &err)
	{
		gDebug->Log("Cannot read from socket (What: %s)", err.what());
	}

	if(length > 0)
	{
		std::istream response(&buffer);
		std::pair<UINT, std::string> pushData;

		response >> pushData.first;
		std::getline(response, pushData.second);

		try_lock_mutex:

		if(pqMutex.try_lock())
		{
			pendingQueue.push(pushData);
			pqMutex.unlock();
		}
		else
		{
			gDebug->Log("Cannot lock pending queue mutex, continuing anyway");
			boost::this_thread::sleep_for(boost::chrono::milliseconds(1));
			goto try_lock_mutex;
		}
	}
	else
	{
		gDebug->Log("Invalid length (%i) returned by socket read function", length);
	}
}



void addonCore::queueOUT(boost::shared_ptr<boost::asio::ip::tcp::socket> sock)
{
	boost::asio::streambuf buffer;
	std::size_t length;

	while(true)
	{
		ouMutex.lock();

		if(outputQueue.empty())
		{
			ouMutex.unlock();

			break;
		}

		std::ostream request(&buffer);
		std::pair<UINT, std::string> sendData = outputQueue.front();
		ouMutex.unlock();

		request << "TCPQUERY CLIENT_CALL" << " " << sendData.first << std::endl;
		request << "DATA" << " " << sendData.second << "\n.\n.\n.";

		try
		{
			length = boost::asio::write(*sock.get(), buffer);
		}
		catch(boost::system::system_error &err)
		{
			gDebug->Log("Cannot write to socket (What: %s)", err.what());
		}

		if(length < 1)
			gDebug->Log("Invalid length (%i) returned by socket write function", length);

		try_lock_mutex:

		if(ouMutex.try_lock())
		{
			pendingQueue.pop();
			ouMutex.unlock();
		}
		else
		{
			gDebug->Log("Cannot lock output queue mutex, continuing anyway");
			boost::this_thread::sleep_for(boost::chrono::milliseconds(1));
			goto try_lock_mutex;
		}

		boost::this_thread::sleep_for(boost::chrono::milliseconds(1));
	}
}



void addonCore::processFunc()
{
	while(true)
	{
		pqMutex.lock();

		if(pendingQueue.empty())
		{
			pqMutex.unlock();

			break;
		}

		std::pair<UINT, std::string> data = pendingQueue.front();
		pqMutex.unlock();

		/*switch(data.first)
		{

		}*/

		try_lock_mutex:

		if(pqMutex.try_lock())
		{
			pendingQueue.pop();
			pqMutex.unlock();
		}
		else
		{
			gDebug->Log("Cannot lock pending queue mutex, continuing anyway");
			boost::this_thread::sleep_for(boost::chrono::milliseconds(1));
			goto try_lock_mutex;
		}

		boost::this_thread::sleep_for(boost::chrono::milliseconds(1));
	}
}



void addonCore::Thread()
{
	assert(gCore->getThreadInstance()->get_id() == boost::this_thread::get_id());

	gDebug->traceLastFunction("addonCore::Thread() at 0x%x", &addonCore::Thread);
	//gDebug->Log("Started Core thread with id 0x%x", gCore->getThreadInstance()->get_thread_info()->id);

	boost::this_thread::sleep_for(boost::chrono::seconds(5));

	while(!gD3Device->getDevice(true)) // Wait until we create D3D device
		boost::this_thread::sleep_for(boost::chrono::seconds(1));

	gD3Device->renderText("Loading SAMP-Addon...", 10, 10, 255, 255, 255, 127);
	gD3Device->renderText("Scanning GTA dir for illegal files...", 10, 30, 255, 255, 255, 127);

	gLoader = boost::shared_ptr<addonLoader>(new addonLoader());

	gD3Device->stopLastRender();
	gD3Device->stopLastRender();
	gD3Device->renderText("Loading SAMP-Addon... OK!", 10, 10, 255, 255, 255, 127);
	gD3Device->renderText("Scanning GTA dir for illegal files... OK!", 10, 30, 255, 255, 255, 127);

	std::string serial = gPool->getVar("playerSerial");
	std::string ip = gPool->getVar("serverIP");
	UINT port = (atoi(gPool->getVar("serverPort").c_str()) + 1);

	gD3Device->renderText(strFormat() << "Your UID is: " << serial, 300, 30, 255, 255, 255, 127);
	gD3Device->renderText(strFormat() << "Connecting to addon TCP server " << ip << ":" << port << " ...", 10, 50, 255, 255, 255, 127);

	if(gSocket->Connect(ip, port))
	{
		gD3Device->stopLastRender();
		gD3Device->renderText(strFormat() << "Connecting to addon TCP server " << ip << ":" << port << " ... OK!", 10, 50, 255, 255, 255, 127);
	}
	else
	{
		gD3Device->stopLastRender();
		gD3Device->renderText(strFormat() << "Connecting to addon TCP server " << ip << ":" << port << " ... ERROR!", 10, 50, 255, 255, 255, 127);
		gD3Device->renderText("Addon TCP server is temporary unavalible, or this server hasn't support of SAMP-Addon", 10, 70, 255, 255, 255, 127);
	}

	while(!GTA_PED_STATUS_ADDR) // PED context load flag
		boost::this_thread::sleep_for(boost::chrono::milliseconds(15));

	gD3Device->clearRender();

	gFunctions->ToggleGrassRendering(true);
	gFunctions->ToggleMotionBlur(true);
	gFunctions->ToggleHUD(false);
	gFunctions->ToggleSeaWays(true);

	while(true)
	{
		boost::this_thread::disable_interruption di;

		gCore->processFunc();

		boost::this_thread::restore_interruption re(di);
		boost::this_thread::sleep_for(boost::chrono::milliseconds(50));
	}
}