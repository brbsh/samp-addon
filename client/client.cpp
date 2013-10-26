#pragma once



#include "client.h"





HINSTANCE d3d9_dll_instance;
HINSTANCE d3d9_original;
FARPROC proxy = NULL;

IDirect3D9 *render_original;
ProxyIDirect3D9 *render_hooked;
IDirect3DDevice9 *device_hooked;
ProxyIDirect3DDevice9 *device_original;

boost::atomic<bool> addonInit;


extern boost::shared_ptr<addonCore> gCore;
extern boost::shared_ptr<addonD3Device> gD3Device;
extern boost::shared_ptr<addonDebug> gDebug;





BOOL APIENTRY DllMain(HMODULE hModule, DWORD ul_reason_for_call, LPVOID lpReserved)
{
	//gDebug->TraceLastFunction(strFormat() << "DllMain(hModule = 0x" << std::hex << hModule << ", ul_reason_for_call = 0x" << std::hex << ul_reason_for_call << ", lpReserved = 0x" << std::hex << lpReserved << ") at 0x" << std::hex << &DllMain);

	switch(ul_reason_for_call)
	{
		case DLL_PROCESS_ATTACH:
		{
			if(GetModuleHandle("d3d9.dll") != hModule)
				return false;

			char filename[MAX_PATH];

			DisableThreadLibraryCalls(hModule);
			SetUnhandledExceptionFilter(addonDebug::UnhandledExceptionFilter);

			gD3Device = boost::shared_ptr<addonD3Device>(new addonD3Device());

			GetSystemDirectory(filename, (UINT)(MAX_PATH - 10));
			strcat_s(filename, "\\d3d9.dll");

			d3d9_dll_instance = hModule;
			d3d9_original = LoadLibrary(filename);

			if(!d3d9_original)
			{
				return false;
			}

			proxy = GetProcAddress(d3d9_original, "Direct3DCreate9");
		}
		break;

		case DLL_PROCESS_DETACH:
		{
			FreeLibrary(d3d9_original);
			d3d9_original = NULL;
		}
		break;
	}

	return true;
}



// Direct3DCreate9
IDirect3D9 *__stdcall d3dHook_Direct3DCreate9(UINT SDKVersion)
{
	if(!addonInit)
		gCore = boost::shared_ptr<addonCore>(new addonCore());

	addonInit = true;

	gDebug->TraceLastFunction(strFormat() << "d3dHook_Direct3DCreate9(SDKVersion = 0x" << std::hex << SDKVersion << ") at 0x" << std::hex << &d3dHook_Direct3DCreate9);

	render_original = ((pfnDirect3DCreate9)proxy)(SDKVersion);
	render_hooked = new ProxyIDirect3D9(render_original);

	gD3Device->SetRender(render_original);

	return render_hooked;
}