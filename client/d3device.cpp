#pragma once



#include "d3device.h"





boost::shared_ptr<addonD3Device> gD3Device;


extern boost::shared_ptr<addonDebug> gDebug;





addonD3Device::addonD3Device()
{
	//gDebug->TraceLastFunction("addonD3Device::addonD3Device() at 0x?????");

	this->OriginalRender = NULL;
	this->OriginalDevice = NULL;
	this->wText = NULL;
}



addonD3Device::~addonD3Device()
{
	gDebug->TraceLastFunction("addonD3Device::~addonD3Device() at 0x?????");
}



void addonD3Device::SetRender(IDirect3D9 *render)
{
	gDebug->TraceLastFunction(strFormat() << "addonD3Device::SetRender(render = 0x" << std::hex << (int)render << ") at 0x" << std::hex << &addonD3Device::SetRender);

	this->OriginalRender = render;
}



IDirect3D9 *addonD3Device::GetRender()
{
	gDebug->TraceLastFunction(strFormat() << "addonD3Device::GetRender() at 0x" << std::hex << &addonD3Device::GetRender);

	return this->OriginalRender;
}



void addonD3Device::SetDevice(IDirect3DDevice9 *device)
{
	gDebug->TraceLastFunction(strFormat() << "addonD3Device::SetDevice(device = 0x" << std::hex << (int)device << ") at 0x" << std::hex << &addonD3Device::SetDevice);

	this->OriginalDevice = device;
}



IDirect3DDevice9 *addonD3Device::GetDevice()
{
	gDebug->TraceLastFunction(strFormat() << "addonD3Device::GetDevice() at 0x" << std::hex << &addonD3Device::GetDevice);

	return this->OriginalDevice;
}



void addonD3Device::Screenshot(std::string filename)
{
	gDebug->TraceLastFunction(strFormat() << "addonD3Device::Screenshot(filename = '" << filename << "') at 0x" << std::hex << &addonD3Device::Screenshot);

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
	gDebug->TraceLastFunction(strFormat() << "addonD3Device::InitFontRender() at 0x" << std::hex << &addonD3Device::InitFontRender);

	if(this->wText == NULL)
		D3DXCreateFont(this->OriginalDevice, 18, NULL, FW_NORMAL, NULL, FALSE, DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, DEFAULT_QUALITY, DEFAULT_PITCH, TEXT("Arial"), &this->wText);
	else
	{
		this->wText->OnLostDevice();
		this->wText->OnResetDevice();
	}
}



void addonD3Device::RenderText(std::string text, int x, int y, int r, int g, int b, int a)
{
	gDebug->TraceLastFunction(strFormat() << "addonD3Device::RenderText(...) at 0x" << std::hex << &addonD3Device::RenderText);

	RECT rText;

	rText.left = x;
	rText.top = y;
	rText.right = 10000;
	rText.bottom = 10000;

	this->OriginalDevice->BeginScene();
	this->wText->DrawText(NULL, text.c_str(), text.length(), &rText, NULL, D3DCOLOR_ARGB(a, r, g, b));
	this->OriginalDevice->EndScene();
}



void addonD3Device::ReleaseRenderedText()
{
	gDebug->TraceLastFunction(strFormat() << "addonD3Device::ReleaseRenderedText() at 0x" << std::hex << &addonD3Device::ReleaseRenderedText);

	this->OriginalDevice->BeginScene();
	this->wText->Release();
	this->OriginalDevice->EndScene();
}