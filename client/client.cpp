#pragma once



#include "client.h"





int hookCount = NULL;
bool sampLoaded = false;

HINSTANCE addonDLLInstance = NULL;
HINSTANCE d3d9DLLInstance = NULL;

FARPROC proxy = NULL;


extern boost::shared_ptr<addonCore> gCore;
extern boost::shared_ptr<addonD3Device> gD3Device;
extern boost::shared_ptr<addonDebug> gDebug;


void singlePlayerRender();





BOOL WINAPI DllMain(HMODULE hModule, DWORD ul_reason_for_call, LPVOID lpReserved)
{
	switch(ul_reason_for_call)
	{
		case DLL_PROCESS_ATTACH:
		{
			if(GetModuleHandle("d3d9.dll") != hModule)
				return false;

			if(!GetModuleHandle("gta_sa.exe") && !GetModuleHandle("gta-sa.exe"))
				return false;

			sampLoaded = !(!GetModuleHandle("samp.dll"));

			DisableThreadLibraryCalls(hModule);

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
			if(d3d9DLLInstance)
			{
				FreeLibrary(d3d9DLLInstance);
				d3d9DLLInstance = NULL;
			}

			if(hookCount > 0)
				exit(EXIT_SUCCESS);
		}
		break;
	}

	return true;
}



// Direct3DCreate9
IDirect3D9 *WINAPI d3dHook_Direct3DCreate9(UINT SDKVersion)
{
	IDirect3D9 *render_original = NULL;
	ProxyIDirect3D9 *render_hooked = NULL;

	hookCount++;

	if(sampLoaded)
	{
		if(hookCount == 1)
		{
			gDebug = boost::shared_ptr<addonDebug>(new addonDebug());
			gD3Device = boost::shared_ptr<addonD3Device>(new addonD3Device());
			gCore = boost::shared_ptr<addonCore>(new addonCore());

			gDebug->traceLastFunction("d3dHook_Direct3DCreate9(SDKVersion = 0x%x) at 0x%x", SDKVersion, &d3dHook_Direct3DCreate9);
			gDebug->Log("Called Direct3DCreate9, hooking it...");
		}
	}
	else
	{
		if(hookCount == 1)
		{
			gDebug = boost::shared_ptr<addonDebug>(new addonDebug());
			gD3Device = boost::shared_ptr<addonD3Device>(new addonD3Device());
		}
		else if(hookCount == 2)
		{
			MessageBox(NULL, "SA-MP isn't launching, forcing singleplayer mode\n\nSA-MP не запущен, выбран режим одиночной игры", "SAMP-Addon", NULL);

			boost::thread render(&singlePlayerRender);
		}
	}

	render_original = ((pfnDirect3DCreate9)proxy)(SDKVersion);
	render_hooked = new ProxyIDirect3D9(render_original);
	
	if(hookCount == 1)
	{
		gD3Device->setRender(render_original, true);
		gD3Device->setRender(render_hooked, false);

		gDebug->Log("Original render address: 0x%x", (int)render_original);
		gDebug->Log("Hooked render address: 0x%x", (int)render_hooked);
		gDebug->Log("Returning hooked render address, hook was installed!");
	}

	return render_hooked;
}



void singlePlayerRender()
{
	gDebug->traceLastFunction("singlePlayerRender() at 0x%x", &addonD3Device::Screenshot);

	while(!gD3Device->getDevice(true)) // Wait until we create D3D device
		boost::this_thread::sleep_for(boost::chrono::seconds(1));

	gD3Device->renderText("SAMP-Addon started in singleplayer mode", 10, 10, 255, 255, 255, 127);

	boost::this_thread::sleep_for(boost::chrono::seconds(5));

	gD3Device->stopLastRender();
}