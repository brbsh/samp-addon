#pragma once



#include "core.h"





boost::shared_ptr<addonCore> gCore;


extern boost::shared_ptr<addonD3Device> gD3Device;
extern boost::shared_ptr<addonDebug> gDebug;
extern boost::shared_ptr<addonFunctions> gFunctions;
extern boost::shared_ptr<addonLoader> gLoader;
extern boost::shared_ptr<addonSocket> gSocket;





addonCore::addonCore()
{
	gDebug = boost::shared_ptr<addonDebug>(new addonDebug());
	gFunctions = boost::shared_ptr<addonFunctions>(new addonFunctions());
	gSocket = boost::shared_ptr<addonSocket>(new addonSocket());
	gLoader = boost::shared_ptr<addonLoader>(new addonLoader());

	gDebug->traceLastFunction("addonCore::addonCore() at 0x?????");

	this->mutexInstance = boost::shared_ptr<boost::mutex>(new boost::mutex());
	this->threadInstance = boost::shared_ptr<boost::thread>(new boost::thread(boost::bind(&addonCore::Thread)));
}



addonCore::~addonCore()
{
	//gFunctions->~addonFunctions();
	//gFunctions.reset();

	//gSocket->~addonSocket();
	//gSocket.reset();

	//gLoader->~addonLoader();
	//gLoader.reset();

	gDebug->traceLastFunction("addonCore::~addonCore() at 0x?????");

	this->getMutexInstance()->destroy();
	this->getThreadInstance()->interrupt();
}



void addonCore::Thread()
{
	assert(gCore->getThreadInstance()->get_id() == boost::this_thread::get_id());

	boost::mutex tMutex;

	gDebug->traceLastFunction("addonCore::Thread() at 0x%x", &addonCore::Thread);

	while(gD3Device->getDevice(true) == NULL) // Wait until we create D3D device
		boost::this_thread::sleep_for(boost::chrono::seconds(1));

	renderData loading;

	loading.text.assign("Loading SAMP-Addon...");
	loading.x = 10;
	loading.y = 10;
	loading.r = 255;
	loading.g = 0;
	loading.b = 0;
	loading.a = 127;

	tMutex.lock();
	gD3Device->renderList.push_back(loading);
	tMutex.unlock();

	boost::this_thread::sleep_for(boost::chrono::seconds(2));

	tMutex.lock();
	gD3Device->renderList.pop_back();
	tMutex.unlock();

	loading.text.assign("Loading SAMP-Addon... OK!");
	loading.x = 10;
	loading.y = 10;
	loading.r = 255;
	loading.g = 0;
	loading.b = 0;
	loading.a = 127;

	tMutex.lock();
	gD3Device->renderList.push_back(loading);
	tMutex.unlock();

	loading.text.assign("Scanning GTA dir for illegal files...");
	loading.x = 10;
	loading.y = 30;
	loading.r = 255;
	loading.g = 0;
	loading.b = 0;
	loading.a = 127;

	tMutex.lock();
	gD3Device->renderList.push_back(loading);
	tMutex.unlock();

	boost::this_thread::sleep_for(boost::chrono::seconds(2));

	tMutex.lock();
	gD3Device->renderList.pop_back();
	tMutex.unlock();

	loading.text.assign("Scanning GTA dir for illegal files... OK!");
	loading.x = 10;
	loading.y = 30;
	loading.r = 255;
	loading.g = 0;
	loading.b = 0;
	loading.a = 127;

	tMutex.lock();
	gD3Device->renderList.push_back(loading);
	tMutex.unlock();

	std::string cmdline(GetCommandLine());

	std::size_t name_ptr = (cmdline.find("-c -n") + 6);
	std::size_t ip_ptr = (cmdline.find("-h") + 3);
	std::size_t port_ptr = (cmdline.find("-p") + 3);
	std::size_t pass_ptr = (cmdline.find("-z") + 3);

	std::string name = cmdline.substr(name_ptr, (ip_ptr - name_ptr - 4));
	std::string ip = cmdline.substr(ip_ptr, (port_ptr - ip_ptr - 4));
	UINT port = atoi(cmdline.substr(port_ptr, 5).c_str());

	if(pass_ptr != std::string::npos)
	{
		//password
	}

	loading.text.assign(strFormat() << "Connecting to " << ip << ":" << port << " ...");
	loading.x = 10;
	loading.y = 50;
	loading.r = 255;
	loading.g = 0;
	loading.b = 0;
	loading.a = 127;

	tMutex.lock();
	gD3Device->renderList.push_back(loading);
	tMutex.unlock();

	//gSocket->Connect("127.0.0.1", 7777);

	while(GTA_PED_STATUS_ADDR == NULL) // PED context load flag
		boost::this_thread::sleep_for(boost::chrono::milliseconds(15));

	tMutex.lock();
	gD3Device->renderList.clear();
	tMutex.unlock();

	boost::this_thread::sleep_for(boost::chrono::seconds(10));

	gFunctions->ToggleMotionBlur(true);
	gFunctions->ToggleGrassRendering(true);

	//RaiseException(0x0000DEAD, NULL, NULL, NULL);
}