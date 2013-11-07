#pragma once



#include "d3device.h"





boost::shared_ptr<addonD3Device> gD3Device;


extern boost::shared_ptr<addonDebug> gDebug;





addonD3Device::addonD3Device()
{
	//gDebug->traceLastFunction("addonD3Device::addonD3Device() at 0x?????");

	this->originalRender = NULL;
	this->hookedRender = NULL;
	this->originalDevice = NULL;
	this->hookedDevice = NULL;

	this->renderInstance = NULL;

	this->mutexInstance = boost::shared_ptr<boost::mutex>(new boost::mutex());
	//this->threadInstance = boost::shared_ptr<boost::thread>(new boost::thread(boost::bind(&addonD3Device::Thread)));
}



addonD3Device::~addonD3Device()
{
	gDebug->traceLastFunction("addonD3Device::~addonD3Device() at 0x?????");
	gDebug->Log("Called D3Device destructor");

	this->mutexInstance->destroy();
}



void addonD3Device::setRender(IDirect3D9 *render, bool original)
{
	//gDebug->traceLastFunction("addonD3Device::setRender(render = 0x%x, original = %s) at 0x%x", (int)render, ((original) ? ("true") : ("false")), &addonD3Device::setRender);

	this->mutexInstance->lock();

	if(original)
		this->originalRender = render;
	else
		this->hookedRender = render;

	this->mutexInstance->unlock();
}



void addonD3Device::setDevice(IDirect3DDevice9 *device, bool original)
{
	//gDebug->traceLastFunction("addonD3Device::setDevice(device = 0x%x) at 0x%x", (int)device, ((original) ? ("true") : ("false")), &addonD3Device::setDevice);

	this->mutexInstance->lock();

	if(original)
		this->originalDevice = device;
	else
		this->hookedDevice = device;

	this->mutexInstance->unlock();
}



void addonD3Device::Screenshot(std::string filename)
{
	gDebug->traceLastFunction("addonD3Device::Screenshot(filename = '%s') at 0x%x", filename.c_str(), &addonD3Device::Screenshot);

	boost::filesystem::path file(filename);

	if(boost::filesystem::exists(file))
	{
		boost::system::error_code error;

		boost::filesystem::remove(file, error);

		if(error)
			gDebug->Log("Cannot remove file %s: %s (Error code: %i)", filename.c_str(), error.message().c_str(), error.value());
	}

	IDirect3DSurface9 *pSurface;

	this->originalDevice->CreateOffscreenPlainSurface(GetSystemMetrics(SM_CXSCREEN), GetSystemMetrics(SM_CYSCREEN), D3DFMT_A8R8G8B8, D3DPOOL_SYSTEMMEM, &pSurface, NULL);
	this->originalDevice->GetBackBuffer(NULL, NULL, D3DBACKBUFFER_TYPE_MONO, &pSurface);

	D3DXSaveSurfaceToFile(filename.c_str(), D3DXIFF_PNG, pSurface, NULL, NULL);

	pSurface->Release();
}



void addonD3Device::initRender(boost::mutex *mutex)
{
	mutex->lock();

	if(!this->renderInstance)
	{
		D3DXCreateFont(this->hookedDevice, 18, NULL, FW_BOLD, NULL, FALSE, DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, DEFAULT_QUALITY, DEFAULT_PITCH, TEXT("Arial"), &this->renderInstance);
	}
	else
	{
		this->renderInstance->OnLostDevice();
		this->renderInstance->OnResetDevice();
	}

	mutex->unlock();
}



void addonD3Device::renderText(std::string text, int x, int y, int r, int g, int b, int a, boost::mutex *mutex)
{
	gDebug->traceLastFunction("addonD3Device::renderText(text = '%s', x = 0x%x, y = 0x%x, r = 0x%x, g = 0x%x, b = 0x%x, a = 0x%x) at 0x%x", text.c_str(), x, y, r, g, b, a, &addonD3Device::renderText);

	renderData struct_push;

	struct_push.text = text;
	struct_push.x = x;
	struct_push.y = y;
	struct_push.r = r;
	struct_push.g = g;
	struct_push.b = b;
	struct_push.a = a;

	mutex->lock();
	this->renderList.push_back(struct_push);
	mutex->unlock();
}



void addonD3Device::stopLastRender(boost::mutex *mutex)
{
	gDebug->traceLastFunction("addonD3Device::stopLastRender() at 0x%x", &addonD3Device::stopLastRender);

	mutex->lock();
	this->renderList.pop_back();
	mutex->unlock();
}



void addonD3Device::clearRender(boost::mutex *mutex)
{
	gDebug->traceLastFunction("addonD3Device::clearRender() at 0x%x", &addonD3Device::clearRender);

	mutex->lock();
	this->renderList.clear();
	mutex->unlock();
}