#pragma once



#include "client.h"





struct renderData
{
	std::string text;
	int x;
	int y;
	int r;
	int g;
	int b;
	int a;
};



class addonD3Device
{

public:

	std::list<renderData> renderList;

	addonD3Device();
	virtual ~addonD3Device();

	void Screenshot(std::string filename);

	void InitFontRender();
	void RenderText(std::string text, int x, int y, int r, int g, int b, int a);

	void setRender(IDirect3D9 *render, bool original);
	void setDevice(IDirect3DDevice9 *device, bool original);

	IDirect3D9 *getRender(bool original) const
	{
		return (original) ? OriginalRender : HookedRender;
	}

	IDirect3DDevice9 *getDevice(bool original) const
	{
		return (original) ? OriginalDevice : HookedDevice;
	}

	boost::mutex *getMutexInstance() const
	{
		return mutexInstance.get();
	}

private:

	IDirect3D9 *OriginalRender;
	IDirect3D9 *HookedRender;
	IDirect3DDevice9 *OriginalDevice;
	IDirect3DDevice9 *HookedDevice;

	ID3DXFont *wText;

	boost::shared_ptr<boost::mutex> mutexInstance;
	boost::shared_ptr<boost::thread> threadInstance;
};