#pragma once



#include "client.hpp"





class addonFunctions
{

public:

	addonFunctions();
	virtual ~addonFunctions();

	void ToggleMotionBlur(bool toggle);
	void ToggleGrassRendering(bool toggle);
	void ToggleHUD(bool toggle);
	void ToggleSeaWays(bool toggle);

	void SetGameSpeed(float speed);
	float GetGameSpeed();

private:

	DWORD protFlags;
};