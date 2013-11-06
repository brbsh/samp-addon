#pragma once



#include "d3device.h"





boost::shared_ptr<addonD3Device> gD3Device;


extern boost::shared_ptr<addonDebug> gDebug;





addonD3Device::addonD3Device()
{
	//gDebug->traceLastFunction("addonD3Device::addonD3Device() at 0x?????");

	this->OriginalRender = NULL;
	this->HookedRender = NULL;
	this->OriginalDevice = NULL;
	this->HookedDevice = NULL;

	this->wText = NULL;

	this->mutexInstance = boost::shared_ptr<boost::mutex>(new boost::mutex());
	//this->threadInstance = boost::shared_ptr<boost::thread>(new boost::thread(boost::bind(&addonD3Device::Thread)));
}



addonD3Device::~addonD3Device()
{
	gDebug->traceLastFunction("addonD3Device::~addonD3Device() at 0x?????");

	this->wText->Release();
	this->mutexInstance->destroy();
}



void addonD3Device::setRender(IDirect3D9 *render, bool original)
{
	//gDebug->traceLastFunction("addonD3Device::setRender(render = 0x%x, original = %s) at 0x%x", (int)render, ((original) ? ("true") : ("false")), &addonD3Device::setRender);

	this->mutexInstance->lock();

	if(original)
		this->OriginalRender = render;
	else
		this->HookedRender = render;

	this->mutexInstance->unlock();
}



void addonD3Device::setDevice(IDirect3DDevice9 *device, bool original)
{
	//gDebug->traceLastFunction("addonD3Device::setDevice(device = 0x%x) at 0x%x", (int)device, ((original) ? ("true") : ("false")), &addonD3Device::setDevice);

	this->mutexInstance->lock();

	if(original)
		this->OriginalDevice = device;
	else
		this->HookedDevice = device;

	this->mutexInstance->unlock();
}



void addonD3Device::Screenshot(std::string filename)
{
	gDebug->traceLastFunction("addonD3Device::Screenshot(filename = '%s') at 0x%x", filename.c_str(), &addonD3Device::Screenshot);

	boost::filesystem::path file(filename);

	if(boost::filesystem::exists(file))
		boost::filesystem::remove(file);

	IDirect3DSurface9 *pSurface;

	this->OriginalDevice->CreateOffscreenPlainSurface(GetSystemMetrics(SM_CXSCREEN), GetSystemMetrics(SM_CYSCREEN), D3DFMT_A8R8G8B8, D3DPOOL_SYSTEMMEM, &pSurface, NULL);
	this->OriginalDevice->GetBackBuffer(NULL, NULL, D3DBACKBUFFER_TYPE_MONO, &pSurface);

	D3DXSaveSurfaceToFile(filename.c_str(), D3DXIFF_PNG, pSurface, NULL, NULL);

	pSurface->Release();
}



void addonD3Device::InitFontRender()
{
	//gDebug->traceLastFunction("addonD3Device::InitFontRender() at 0x%x", &addonD3Device::InitFontRender);

	if(this->wText == NULL)
		D3DXCreateFont(this->HookedDevice, 18, NULL, FW_BOLD, NULL, FALSE, DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, DEFAULT_QUALITY, DEFAULT_PITCH, TEXT("Arial"), &this->wText);
	else
	{
		this->wText->OnLostDevice();
		this->wText->OnResetDevice();
	}
}



void addonD3Device::RenderText(std::string text, int x, int y, int r, int g, int b, int a)
{
	//gDebug->traceLastFunction("addonD3Device::RenderText(text = '%s', x = 0x%x, y = 0x%x, r = 0x%x, g = 0x%x, b = 0x%x, a = 0x%x) at 0x%x", text.c_str(), x, y, r, g, b, a, &addonD3Device::RenderText);

	RECT rText;

	rText.left = x;
	rText.top = y;
	rText.right = 1680;
	rText.bottom = (y + 200);

	this->wText->DrawText(NULL, text.c_str(), -1, &rText, NULL, D3DCOLOR_ARGB(a, r, g, b));
}