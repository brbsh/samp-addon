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

	void initRender();
	void processRender();
	void renderText(std::string text, int x, int y, int r, int g, int b, int a);
	void stopLastRender();
	void clearRender();

	void setRender(IDirect3D9 *render, bool original);
	void setDevice(IDirect3DDevice9 *device, bool original);

	IDirect3D9 *getRender(bool original)
	{
		boost::shared_lock<boost::shared_mutex> lockit(renMutex);
		return (original) ? originalRender : hookedRender;
	}

	IDirect3DDevice9 *getDevice(bool original)
	{
		boost::shared_lock<boost::shared_mutex> lockit(devMutex);
		return (original) ? originalDevice : hookedDevice;
	}

	ID3DXFont *getTextRender()
	{
		boost::shared_lock<boost::shared_mutex> lockit(txtMutex);
		return renderInstance;
	}

private:

	IDirect3D9 *originalRender;
	IDirect3D9 *hookedRender;
	IDirect3DDevice9 *originalDevice;
	IDirect3DDevice9 *hookedDevice;

	ID3DXFont *renderInstance;

	boost::mutex d3Mutex;
	boost::shared_mutex renMutex;
	boost::shared_mutex devMutex;
	boost::shared_mutex txtMutex;
};