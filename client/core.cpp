#pragma once



#include "core.h"





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

	this->threadInstance = boost::shared_ptr<boost::thread>(new boost::thread(boost::bind(&addonCore::Thread)));
}



addonCore::~addonCore()
{
	gDebug->traceLastFunction("addonCore::~addonCore() at 0x?????");
	gDebug->Log("Core destructor called");

	this->threadInstance->interruption_requested();
}



void addonCore::Queue(std::pair<UINT, std::string> set)
{
	boost::unique_lock<boost::mutex> lockit(this->pqMutex);
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

	while(true)
	{
		while(!gCore->pendingQueue.empty())
		{
			boost::this_thread::disable_interruption di;

			std::pair<UINT, std::string> data;

			boost::unique_lock<boost::mutex> lockit(gCore->pqMutex);
			data = gCore->pendingQueue.front();
			gCore->pendingQueue.pop();
			lockit.unlock();

			/*switch(data.first)
			{

			}*/

			boost::this_thread::restore_interruption re(di);
			boost::this_thread::sleep_for(boost::chrono::milliseconds(1));
		}

		boost::this_thread::sleep_for(boost::chrono::milliseconds(50));
	}
}