#pragma once



#include "client.hpp"





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

	threadInstance->interrupt();
	threadInstance.reset();

	gLoader.reset();
	gSocket.reset();
	gPool.reset();
	gFunctions.reset();
}



void addonCore::queueIN(addonSocket *instance)
{
	boost::array<char, 2048> buffer;
	std::size_t bytes_rx;

	try
	{
		bytes_rx = instance->getSocket().read_some(boost::asio::buffer(buffer));
	}
	catch(boost::system::system_error& error)
	{
		gDebug->Log("Cannot read from socket (What: %s)", error.what());
		gSocket->disconnect();

		return;
	}

	if((bytes_rx > 0) && (bytes_rx < buffer.max_size()))
	{
		std::string data(buffer.data());
		data.erase(bytes_rx, INFINITE);

		gDebug->Log("Got '%s' (length: %i) from server", data.c_str(), bytes_rx);

		try_lock_mutex:

		if(pqMutex.try_lock())
		{
			//pendingQueue.push(pushData);
			pqMutex.unlock();
		}
		else
		{
			gDebug->Log("Cannot lock pending queue mutex, continuing anyway");
			boost::this_thread::yield();
			goto try_lock_mutex;
		}
	}
	else
	{
		gDebug->Log("Invalid length (%i) returned by socket read function", bytes_rx);

		gSocket->disconnect();
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

		std::pair<unsigned int, std::string> data = pendingQueue.front();
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
			boost::this_thread::yield();
			goto try_lock_mutex;
		}

		boost::this_thread::yield();
	}
}



void addonCore::Thread()
{
	assert(gCore->getThreadInstance()->get_id() == boost::this_thread::get_id());

	gDebug->traceLastFunction("addonCore::Thread() at 0x%x", &addonCore::Thread);
	gDebug->Log("Started Core thread");

	boost::this_thread::sleep(boost::posix_time::seconds(5)); //boost::this_thread::sleep_for(boost::chrono::seconds(5));

	while(!gD3Device->getDevice(true)) // Wait until we create D3D device
		boost::this_thread::sleep(boost::posix_time::seconds(1)); //boost::this_thread::sleep_for(boost::chrono::seconds(1));

	gD3Device->renderText("Loading SAMP-Addon...", 10, 10, 255, 255, 255, 127);
	gD3Device->renderText("Scanning GTA dir for illegal files...", 10, 30, 255, 255, 255, 127);

	gLoader = boost::shared_ptr<addonLoader>(new addonLoader());

	gD3Device->stopLastRender();
	gD3Device->stopLastRender();
	gD3Device->renderText("Loading SAMP-Addon... OK!", 10, 10, 255, 255, 255, 127);
	gD3Device->renderText("Scanning GTA dir for illegal files... OK!", 10, 30, 255, 255, 255, 127);

	std::string serial = gPool->getVar("serial");

	gD3Device->renderText(strFormat() << "Your UID is: " << serial, 300, 30, 255, 255, 255, 127);

	while(!GTA_PED_STATUS_ADDR) // PED context load flag
		boost::this_thread::sleep(boost::posix_time::milliseconds(250)); ///boost::this_thread::sleep_for(boost::chrono::milliseconds(15));

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
		boost::this_thread::sleep(boost::posix_time::milliseconds(10)); //boost::this_thread::sleep_for(boost::chrono::milliseconds(50));
	}
}