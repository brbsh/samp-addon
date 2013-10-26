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
	gSocket = boost::shared_ptr<addonSocket>(new addonSocket());
	gLoader = boost::shared_ptr<addonLoader>(new addonLoader());

	gDebug->TraceLastFunction("addonCore::addonCore() at 0x?????");

	this->mutexInstance = boost::shared_ptr<boost::mutex>(new boost::mutex());
	this->threadInstance = boost::shared_ptr<boost::thread>(new boost::thread(boost::bind(&addonCore::Thread)));
}



addonCore::~addonCore()
{
	gDebug->TraceLastFunction("addonCore::~addonCore() at 0x?????");

	this->getMutexInstance()->destroy();
	this->getThreadInstance()->interrupt();
}



boost::mutex *addonCore::getMutexInstance()
{
	gDebug->TraceLastFunction(strFormat() << "addonCore::getMutexInstance() at 0x" << std::hex << &addonCore::getMutexInstance);

	return this->mutexInstance.get();
}



boost::thread *addonCore::getThreadInstance()
{
	gDebug->TraceLastFunction(strFormat() << "addonCore::getThreadInstance() at 0x" << std::hex << &addonCore::getThreadInstance);

	return this->threadInstance.get();
}



void addonCore::Thread()
{
	gDebug->TraceLastFunction(strFormat() << "addonCore::Thread() at 0x" << std::hex << &addonCore::Thread);

	while(gD3Device->GetDevice() == NULL) // Wait until we create D3D device
		boost::this_thread::sleep_for(boost::chrono::seconds(1));

	//gD3Device->InitFontRender();
	//gD3Device->RenderText("SAMP-Addon by BJIADOKC started", 10, 100, 255, 0, 0, 255); // Crashes client. Needs an separated render instance
	//gD3Device->ReleaseRenderedText();

	while(*(int *)0xB6F5F0 == 0) // PED context load flag
		boost::this_thread::sleep_for(boost::chrono::milliseconds(100));

	boost::this_thread::sleep_for(boost::chrono::seconds(10));

	gFunctions = boost::shared_ptr<addonFunctions>(new addonFunctions());
	gFunctions->ToggleMotionBlur(true);
	gFunctions->ToggleGrassRendering(true);

	//RaiseException(0x0000DEAD, NULL, NULL, NULL);
}