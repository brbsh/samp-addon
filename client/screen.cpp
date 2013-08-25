#pragma once



#include "screen.h"





addonScreen *gScreen;





addonScreen::addonScreen()
{
	addonDebug("Screen constructor called");

	this->Address = 0x5D0820;
}



addonScreen::~addonScreen()
{
	addonDebug("Screen deconstructor called");
}



void addonScreen::Get(std::string filename)
{
	if(boost::filesystem::exists(boost::filesystem::path(filename)))
		boost::filesystem::remove(boost::filesystem::path(filename));

	DWORD addr = this->Address;
	char *c_filename = new char[filename.length()];

	strcpy(c_filename, filename.c_str());

	_asm
	{
		mov eax, [0xB6F97C]
		mov eax, [eax]
		push c_filename
		push eax
		call addr
		add esp, 8
	}

	delete[] c_filename;
}
