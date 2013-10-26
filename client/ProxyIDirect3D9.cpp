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

HRESULT __stdcall ProxyIDirect3D9::QueryInterface(REFIID riid, void** ppvObj)
{
    *ppvObj = NULL;

	HRESULT hRes = original->QueryInterface(riid, ppvObj); 
	if (hRes == S_OK)
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
	if (count == 0) 
	{
  	    delete(this); 
	}

	return(count);
}

HRESULT __stdcall ProxyIDirect3D9::RegisterSoftwareDevice(void* pInitializeFunction)
{
    return original->RegisterSoftwareDevice(pInitializeFunction);
}

UINT __stdcall ProxyIDirect3D9::GetAdapterCount(void)
{
    return original->GetAdapterCount();
}

HRESULT __stdcall ProxyIDirect3D9::GetAdapterIdentifier(UINT Adapter,DWORD Flags,D3DADAPTER_IDENTIFIER9* pIdentifier)
{
    return original->GetAdapterIdentifier(Adapter,Flags,pIdentifier);
}

UINT __stdcall ProxyIDirect3D9::GetAdapterModeCount(UINT Adapter, D3DFORMAT Format)
{
    return original->GetAdapterModeCount(Adapter, Format);
}

HRESULT __stdcall ProxyIDirect3D9::EnumAdapterModes(UINT Adapter,D3DFORMAT Format,UINT Mode,D3DDISPLAYMODE* pMode)
{
    return original->EnumAdapterModes(Adapter,Format,Mode,pMode);
}

HRESULT __stdcall ProxyIDirect3D9::GetAdapterDisplayMode( UINT Adapter,D3DDISPLAYMODE* pMode)
{
    return original->GetAdapterDisplayMode(Adapter,pMode);
}

HRESULT __stdcall ProxyIDirect3D9::CheckDeviceType(UINT iAdapter,D3DDEVTYPE DevType,D3DFORMAT DisplayFormat,D3DFORMAT BackBufferFormat,BOOL bWindowed)
{
    return original->CheckDeviceType(iAdapter,DevType,DisplayFormat,BackBufferFormat,bWindowed);
}

HRESULT __stdcall ProxyIDirect3D9::CheckDeviceFormat(UINT Adapter,D3DDEVTYPE DeviceType,D3DFORMAT AdapterFormat,DWORD Usage,D3DRESOURCETYPE RType,D3DFORMAT CheckFormat)
{
    return original->CheckDeviceFormat(Adapter,DeviceType,AdapterFormat,Usage,RType,CheckFormat);
}

HRESULT __stdcall ProxyIDirect3D9::CheckDeviceMultiSampleType(UINT Adapter,D3DDEVTYPE DeviceType,D3DFORMAT SurfaceFormat,BOOL Windowed,D3DMULTISAMPLE_TYPE MultiSampleType,DWORD* pQualityLevels)
{
    return original->CheckDeviceMultiSampleType(Adapter,DeviceType,SurfaceFormat,Windowed,MultiSampleType,pQualityLevels);
}

HRESULT __stdcall ProxyIDirect3D9::CheckDepthStencilMatch(UINT Adapter,D3DDEVTYPE DeviceType,D3DFORMAT AdapterFormat,D3DFORMAT RenderTargetFormat,D3DFORMAT DepthStencilFormat)
{
    return original->CheckDepthStencilMatch(Adapter,DeviceType,AdapterFormat,RenderTargetFormat,DepthStencilFormat);
}

HRESULT __stdcall ProxyIDirect3D9::CheckDeviceFormatConversion(UINT Adapter,D3DDEVTYPE DeviceType,D3DFORMAT SourceFormat,D3DFORMAT TargetFormat)
{
    return original->CheckDeviceFormatConversion(Adapter,DeviceType,SourceFormat,TargetFormat);
}

HRESULT __stdcall ProxyIDirect3D9::GetDeviceCaps(UINT Adapter,D3DDEVTYPE DeviceType,D3DCAPS9* pCaps)
{
    return original->GetDeviceCaps(Adapter,DeviceType,pCaps);
}

HMONITOR __stdcall ProxyIDirect3D9::GetAdapterMonitor(UINT Adapter)
{
    return original->GetAdapterMonitor(Adapter);
}

HRESULT __stdcall ProxyIDirect3D9::CreateDevice(UINT Adapter,D3DDEVTYPE DeviceType,HWND hFocusWindow,DWORD BehaviorFlags,D3DPRESENT_PARAMETERS* pPresentationParameters,IDirect3DDevice9** ppResult)
{
	// Look for 'NVIDIA PerfHUD' adapter
	// If it is present, override default settings

	/*todo: experimental code to enable NVPerfHUD, doesn't seem to work tho.

	char buf[1024]; 

	char dir[1024];
	GetCurrentDirectoryA(1024, dir);
	sprintf(buf, "Dir: %s\n", dir);
	OutputDebugStringA(buf);

	sprintf(buf, "Searching %d adapters\n", original->GetAdapterCount()); 
	OutputDebugStringA(buf);

	for (UINT adapterIndex=0;adapterIndex<original->GetAdapterCount();adapterIndex++) 
	{
		D3DADAPTER_IDENTIFIER9  Identifier;
		original->GetAdapterIdentifier(Adapter,0,&Identifier);

		sprintf(buf, "Adapter[%d]: %s\n", adapterIndex, Identifier.Description); 
		OutputDebugStringA(buf);

		if (strstr(Identifier.Description,"PerfHUD") != 0)
		{
			Adapter = adapterIndex;
			DeviceType = D3DDEVTYPE_REF;

			OutputDebugStringA("got nvperhud\n!");
			break;
		}
	}

	DeviceType = D3DDEVTYPE_REF;

	sprintf(buf, "Adapter: %d\n", Adapter); 
	OutputDebugStringA(buf);

	sprintf(buf, "DeviceType: %d\n", DeviceType); 
	OutputDebugStringA(buf);

	s.ShowCallstack(0, 100);
	*/

	IDirect3DDevice9* orginalDevice = NULL;
	HRESULT hres = original->CreateDevice(Adapter, DeviceType, hFocusWindow, BehaviorFlags, pPresentationParameters, &orginalDevice);

	if(!FAILED(hres) && orginalDevice != NULL)
	{
		if(ppResult != NULL)
		{
			*ppResult = new ProxyIDirect3DDevice9(orginalDevice, Adapter, DeviceType, hFocusWindow, BehaviorFlags, pPresentationParameters);
		}
	}
	else if(ppResult != NULL)
	{
		*ppResult = NULL;
	}

	gD3Device->SetDevice(orginalDevice);

	return hres;
}