#pragma once



#include "client.h"





class addonFunctions
{

public:

	addonFunctions();
	virtual ~addonFunctions();

	void ToggleMotionBlur(bool status);
	void ToggleGrassRendering(bool status);

private:

	DWORD protFlags;
};