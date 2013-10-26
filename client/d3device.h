#pragma once



#include "client.h"





class addonD3Device
{

public:

	addonD3Device();
	virtual ~addonD3Device();

	void Screenshot(std::string filename);

	void InitFontRender();
	void RenderText(std::string text, int x, int y, int r, int g, int b, int a);
	void ReleaseRenderedText();

	void SetRender(IDirect3D9 *render);
	IDirect3D9 *GetRender();

	void SetDevice(IDirect3DDevice9 *device);
	IDirect3DDevice9 *GetDevice();

private:

	IDirect3D9 *OriginalRender;
	IDirect3DDevice9 *OriginalDevice;
	ID3DXFont *wText;
};