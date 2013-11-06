#pragma once



#include "ProxyIDirect3D9.h"





extern boost::shared_ptr<addonD3Device> gD3Device;





ProxyIDirect3D9::ProxyIDirect3D9(IDirect3D9 *_original) :
	original(_original)
{

}



ProxyIDirect3D9::~ProxyIDirect3D9()
{

}



HRESULT __stdcall ProxyIDirect3D9::QueryInterface(REFIID riid, void **ppvObj)
{
    *ppvObj = NULL;

	HRESULT hRes = original->QueryInterface(riid, ppvObj);

	if(hRes == S_OK)
	{
		*ppvObj = this;
	}

	return hRes;
}



ULONG __stdcall ProxyIDirect3D9::AddRef(void)
{
    return original->AddRef();
}



ULONG __stdcall ProxyIDirect3D9::Release(void)
{
	ULONG count = original->Release();

	if(count == 0) 
	{
  	    delete(this); 
	}

	return(count);
}



HRESULT __stdcall ProxyIDirect3D9::RegisterSoftwareDevice(void *pInitializeFunction)
{
    return original->RegisterSoftwareDevice(pInitializeFunction);
}



UINT __stdcall ProxyIDirect3D9::GetAdapterCount(void)
{
    return original->GetAdapterCount();
}



HRESULT __stdcall ProxyIDirect3D9::GetAdapterIdentifier(UINT Adapter, DWORD Flags, D3DADAPTER_IDENTIFIER9 *pIdentifier)
{
    return original->GetAdapterIdentifier(Adapter, Flags, pIdentifier);
}



UINT __stdcall ProxyIDirect3D9::GetAdapterModeCount(UINT Adapter, D3DFORMAT Format)
{
    return original->GetAdapterModeCount(Adapter, Format);
}



HRESULT __stdcall ProxyIDirect3D9::EnumAdapterModes(UINT Adapter, D3DFORMAT Format, UINT Mode, D3DDISPLAYMODE *pMode)
{
    return original->EnumAdapterModes(Adapter, Format, Mode, pMode);
}



HRESULT __stdcall ProxyIDirect3D9::GetAdapterDisplayMode(UINT Adapter, D3DDISPLAYMODE *pMode)
{
    return original->GetAdapterDisplayMode(Adapter,pMode);
}



HRESULT __stdcall ProxyIDirect3D9::CheckDeviceType(UINT iAdapter, D3DDEVTYPE DevType, D3DFORMAT DisplayFormat, D3DFORMAT BackBufferFormat, BOOL bWindowed)
{
    return original->CheckDeviceType(iAdapter, DevType, DisplayFormat, BackBufferFormat, bWindowed);
}



HRESULT __stdcall ProxyIDirect3D9::CheckDeviceFormat(UINT Adapter, D3DDEVTYPE DeviceType, D3DFORMAT AdapterFormat, DWORD Usage, D3DRESOURCETYPE RType, D3DFORMAT CheckFormat)
{
    return original->CheckDeviceFormat(Adapter, DeviceType, AdapterFormat, Usage, RType, CheckFormat);
}



HRESULT __stdcall ProxyIDirect3D9::CheckDeviceMultiSampleType(UINT Adapter, D3DDEVTYPE DeviceType, D3DFORMAT SurfaceFormat, BOOL Windowed, D3DMULTISAMPLE_TYPE MultiSampleType, DWORD *pQualityLevels)
{
    return original->CheckDeviceMultiSampleType(Adapter, DeviceType, SurfaceFormat, Windowed, MultiSampleType, pQualityLevels);
}



HRESULT __stdcall ProxyIDirect3D9::CheckDepthStencilMatch(UINT Adapter, D3DDEVTYPE DeviceType, D3DFORMAT AdapterFormat, D3DFORMAT RenderTargetFormat, D3DFORMAT DepthStencilFormat)
{
    return original->CheckDepthStencilMatch(Adapter, DeviceType, AdapterFormat, RenderTargetFormat, DepthStencilFormat);
}



HRESULT __stdcall ProxyIDirect3D9::CheckDeviceFormatConversion(UINT Adapter, D3DDEVTYPE DeviceType, D3DFORMAT SourceFormat, D3DFORMAT TargetFormat)
{
    return original->CheckDeviceFormatConversion(Adapter, DeviceType, SourceFormat, TargetFormat);
}



HRESULT __stdcall ProxyIDirect3D9::GetDeviceCaps(UINT Adapter, D3DDEVTYPE DeviceType, D3DCAPS9 *pCaps)
{
    return original->GetDeviceCaps(Adapter, DeviceType, pCaps);
}



HMONITOR __stdcall ProxyIDirect3D9::GetAdapterMonitor(UINT Adapter)
{
    return original->GetAdapterMonitor(Adapter);
}



HRESULT __stdcall ProxyIDirect3D9::CreateDevice(UINT Adapter, D3DDEVTYPE DeviceType, HWND hFocusWindow, DWORD BehaviorFlags, D3DPRESENT_PARAMETERS *pPresentationParameters, IDirect3DDevice9 **ppResult)
{

	IDirect3DDevice9 *orginalDevice = NULL;
	IDirect3DDevice9 *hookedDevice = NULL;

	HRESULT result = original->CreateDevice(Adapter, DeviceType, hFocusWindow, BehaviorFlags, pPresentationParameters, &orginalDevice);

	if(!FAILED(result) && (orginalDevice != NULL))
	{
		if(ppResult != NULL)
		{
			hookedDevice = new ProxyIDirect3DDevice9(orginalDevice, Adapter, DeviceType, hFocusWindow, BehaviorFlags, pPresentationParameters);
		}
	}
	else if(ppResult != NULL)
	{
		*ppResult = NULL;
	}

	gD3Device->setDevice(orginalDevice, true);
	gD3Device->setDevice(hookedDevice, false);

	*ppResult = hookedDevice;

	return result;
}