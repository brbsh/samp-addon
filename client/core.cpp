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
	gDebug = boost::shared_ptr<addonDebug>(new addonDebug());
	gFunctions = boost::shared_ptr<addonFunctions>(new addonFunctions());
	gPool = boost::shared_ptr<addonPool>(new addonPool());
	gSocket = boost::shared_ptr<addonSocket>(new addonSocket());
	gLoader = boost::shared_ptr<addonLoader>(new addonLoader());

	gDebug->traceLastFunction("addonCore::addonCore() at 0x?????");
	gDebug->Log("Core constructor called");

	this->mutexInstance = boost::shared_ptr<boost::mutex>(new boost::mutex());
	this->threadInstance = boost::shared_ptr<boost::thread>(new boost::thread(boost::bind(&addonCore::Thread)));
}



addonCore::~addonCore()
{
	gDebug->traceLastFunction("addonCore::~addonCore() at 0x?????");
	gDebug->Log("Core destructor called");

	this->threadInstance->interruption_requested();
	this->mutexInstance->destroy();
}



void addonCore::Thread()
{
	assert(gCore->getThreadInstance()->get_id() == boost::this_thread::get_id());

	gDebug->traceLastFunction("addonCore::Thread() at 0x%x", &addonCore::Thread);
	gDebug->Log("Started Core thread with id 0x%x", gCore->getThreadInstance()->get_thread_info()->id);

	boost::this_thread::sleep_for(boost::chrono::seconds(5));

	gCore->getMutexInstance()->lock();

	while(!gD3Device->getDevice(true)) // Wait until we create D3D device
		boost::this_thread::sleep_for(boost::chrono::seconds(1));

	gCore->getMutexInstance()->unlock();
	gD3Device->renderText("Loading SAMP-Addon...", 10, 10, 255, 255, 255, 127, gCore->getMutexInstance());

	boost::this_thread::sleep_for(boost::chrono::seconds(1));

	gD3Device->stopLastRender(gCore->getMutexInstance());
	gD3Device->renderText("Loading SAMP-Addon... OK!", 10, 10, 255, 255, 255, 127, gCore->getMutexInstance());
	gD3Device->renderText("Scanning GTA dir for illegal files...", 10, 30, 255, 255, 255, 127, gCore->getMutexInstance());

	boost::this_thread::sleep_for(boost::chrono::seconds(1));

	gD3Device->stopLastRender(gCore->getMutexInstance());
	gD3Device->renderText("Scanning GTA dir for illegal files... OK!", 10, 30, 255, 255, 255, 127, gCore->getMutexInstance());

	std::string serial = gPool->getVar("playerSerial", gCore->getMutexInstance());
	std::string ip = gPool->getVar("serverIP", gCore->getMutexInstance());
	UINT port = (atoi(gPool->getVar("serverPort", gCore->getMutexInstance()).c_str()) + 1);

	gD3Device->renderText(strFormat() << "Your UID is: " << serial, 300, 30, 255, 255, 255, 127, gCore->getMutexInstance());
	gD3Device->renderText(strFormat() << "Connecting to addon TCP server " << ip << ":" << port << " ...", 10, 50, 255, 255, 255, 127, gCore->getMutexInstance());

	boost::this_thread::sleep_for(boost::chrono::seconds(1));

	if(gSocket->Connect(ip, port))
	{
		gD3Device->stopLastRender(gCore->getMutexInstance());
		gD3Device->renderText(strFormat() << "Connecting to addon TCP server " << ip << ":" << port << " ... OK!", 10, 50, 255, 255, 255, 127, gCore->getMutexInstance());
	}
	else
	{
		gD3Device->stopLastRender(gCore->getMutexInstance());
		gD3Device->renderText(strFormat() << "Connecting to addon TCP server " << ip << ":" << port << " ... ERROR!", 10, 50, 255, 255, 255, 127, gCore->getMutexInstance());
		gD3Device->renderText("Addon TCP server is temporary unavalible, or this server hasn't support of SAMP-Addon", 10, 70, 255, 255, 255, 127, gCore->getMutexInstance());
	}

	while(!GTA_PED_STATUS_ADDR) // PED context load flag
		boost::this_thread::sleep_for(boost::chrono::milliseconds(15));

	gD3Device->clearRender(gCore->getMutexInstance());

	gFunctions->ToggleMotionBlur(true);
	gFunctions->ToggleGrassRendering(true);

	while(true)
	{
		while(!gCore->pendingQueue.empty())
		{
			boost::this_thread::disable_interruption di;

			std::pair<UINT, std::string> data;

			gCore->getMutexInstance()->lock();
			data = gCore->pendingQueue.front();
			gCore->pendingQueue.pop();
			gCore->getMutexInstance()->unlock();

			switch(data.first)
			{

			}

			boost::this_thread::restore_interruption re(di);
			boost::this_thread::sleep_for(boost::chrono::milliseconds(1));
		}

		boost::this_thread::sleep_for(boost::chrono::milliseconds(50));
	}
}