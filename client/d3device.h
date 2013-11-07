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

	void initRender(boost::mutex *mutex);
	void renderText(std::string text, int x, int y, int r, int g, int b, int a, boost::mutex *mutex);
	void stopLastRender(boost::mutex *mutex);
	void clearRender(boost::mutex *mutex);

	void setRender(IDirect3D9 *render, bool original);
	void setDevice(IDirect3DDevice9 *device, bool original);

	IDirect3D9 *getRender(bool original) const
	{
		return (original) ? originalRender : hookedRender;
	}

	IDirect3DDevice9 *getDevice(bool original) const
	{
		return (original) ? originalDevice : hookedDevice;
	}

	ID3DXFont *getTextRender() const
	{
		return renderInstance;
	}

	boost::mutex *getMutexInstance() const
	{
		return mutexInstance.get();
	}

private:

	IDirect3D9 *originalRender;
	IDirect3D9 *hookedRender;
	IDirect3DDevice9 *originalDevice;
	IDirect3DDevice9 *hookedDevice;

	ID3DXFont *renderInstance;

	boost::shared_ptr<boost::mutex> mutexInstance;
	//boost::shared_ptr<boost::thread> threadInstance;
};