#pragma once



#include "client.h"





FARPROC proxy = NULL;


boost::atomic<HINSTANCE> addonDLLInstance;
boost::atomic<HINSTANCE> d3d9DLLInstance;
boost::atomic<bool> addonInit;


extern boost::shared_ptr<addonCore> gCore;
extern boost::shared_ptr<addonD3Device> gD3Device;
extern boost::shared_ptr<addonDebug> gDebug;





BOOL APIENTRY DllMain(HMODULE hModule, DWORD ul_reason_for_call, LPVOID lpReserved)
{
	//gDebug->traceLastFunction("DllMain(hModule = 0x%x, ul_reason_for_call = 0x%x, lpReserved = 0x%x) at 0x%x", hModule, ul_reason_for_call, lpReserved, &DllMain);

	switch(ul_reason_for_call)
	{
		case DLL_PROCESS_ATTACH:
		{
			if(GetModuleHandle("d3d9.dll") != hModule)
				return false;

			if(!GetModuleHandle("gta_sa.exe") && !GetModuleHandle("gta-sa.exe"))
				return false;

			if(!GetModuleHandle("samp.dll"))
			{
				if(!addonInit)
					gD3Device = boost::shared_ptr<addonD3Device>(new addonD3Device());

				addonInit = true;
			}

			DisableThreadLibraryCalls(hModule);
			SetUnhandledExceptionFilter(addonDebug::UnhandledExceptionFilter);

			char filename[MAX_PATH];

			GetSystemDirectory(filename, (UINT)(MAX_PATH - 10));
			strcat_s(filename, "\\d3d9.dll");

			addonDLLInstance = hModule;
			d3d9DLLInstance = LoadLibrary(filename);

			if(!d3d9DLLInstance)
				return false;

			proxy = GetProcAddress(d3d9DLLInstance, "Direct3DCreate9");
		}
		break;

		case DLL_PROCESS_DETACH:
		{
			FreeLibrary(d3d9DLLInstance);

			d3d9DLLInstance = NULL;
		}
		break;
	}

	return true;
}



// Direct3DCreate9
IDirect3D9 *__stdcall d3dHook_Direct3DCreate9(UINT SDKVersion)
{
	IDirect3D9 *render_original = NULL;
	ProxyIDirect3D9 *render_hooked = NULL;

	if(!addonInit)
	{
		gCore = boost::shared_ptr<addonCore>(new addonCore());
		gD3Device = boost::shared_ptr<addonD3Device>(new addonD3Device());

		gDebug->traceLastFunction("d3dHook_Direct3DCreate9(SDKVersion = 0x%x) at 0x%x", SDKVersion, &d3dHook_Direct3DCreate9);
		gDebug->Log("Called Direct3DCreate9, hooking it...");
	}

	render_original = ((pfnDirect3DCreate9)proxy)(SDKVersion);
	render_hooked = new ProxyIDirect3D9(render_original);

	gD3Device->setRender(render_original, true);
	gD3Device->setRender(render_hooked, false);
	
	if(!addonInit)
	{
		gDebug->Log("Original render address: 0x%x", (int)render_original);
		gDebug->Log("Hooked render address: 0x%x", (int)render_hooked);
		gDebug->Log("Returning hooked render address, hook was installed!");
	}

	addonInit = true;

	return render_hooked;
}