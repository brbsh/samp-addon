#pragma once



#include "tcpsocket.h"





boost::shared_ptr<addonSocket> gSocket;


extern boost::shared_ptr<addonDebug> gDebug;





addonSocket::addonSocket()
{
	gDebug->TraceLastFunction("addonSocket::addonSocket() at 0x?????");

	boost::system::error_code error;

	this->IOService.run(error);

	if(error)
		gDebug->Log("Cannot run I/O service: %s (%i)", error.message().c_str(), boost::lexical_cast<int>(error));
}



addonSocket::~addonSocket()
{
	gDebug->TraceLastFunction("addonSocket::~addonSocket() at 0x?????");

	this->IOService.stop();
}