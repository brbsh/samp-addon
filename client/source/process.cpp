#pragma once



#include "process.h"





addonProcess *gProcess;


extern addonThread *gThread;
extern addonMutex *gMutex;
extern addonSocket *gSocket;
extern addonScreen *gScreen;
extern addonFS *gFS;

extern std::queue<std::string> rec_from;





addonProcess::addonProcess()
{
	addonDebug("Process constructor called");

	this->processHandle = gThread->Start(addonProcess::Thread, (void *)GetTickCount());
}



addonProcess::~addonProcess()
{
	addonDebug("Process deconstructor called");

	gThread->Stop(this->processHandle);
}



DWORD addonProcess::Thread(void *lpParam)
{
	addonDebug("Thread addonProcess::Thread(%i) successfuly started", (int)lpParam);

	int call_index;
	std::string data;

	do
	{
		if(!rec_from.empty())
		{
			for(unsigned int i = 0; i < rec_from.size(); i++)
			{
				gMutex->Lock();
				data = rec_from.front();
				rec_from.pop();
				gMutex->unLock();

				sscanf_s(data.c_str(), "%*s %*s %d", &call_index);

				switch(call_index)
				{
					case 1000: // TCPQUERY SERVER_CALL 1000 %i1   ---   Response from initial query | %i1 - your HDD serial
					{
						int rec_serial;

						sscanf_s(data.c_str(), "%*s %*s %*d %d", &rec_serial);

						DWORD serial;
						DWORD flags;

						GetVolumeInformationW(L"C:\\", NULL, NULL, &serial, NULL, &flags, NULL, NULL);

						serial += flags;

						if(rec_serial != serial)
						{
							addonDebug("Invalid server answer, terminating");

							gSocket->Close();
						}

						break;
					}

					case 1001: // TCPQUERY SERVER_CALL 1001 %i1   ---   Calls when error occured while connecting to TCP server | %i1 - error code
					{
						int error_code;

						sscanf_s(data.c_str(), "%*s %*s %*d %d", &error_code);

						if(!error_code)
						{
							addonDebug("TCP server is full");
						}

						break;
					}

					//case 1002:

					case 1003: // TCPQUERY SERVER_CALL 1003 %s1   ---   Screenshot take request from server | %s1 - local file name
					{
						char file_name[256];

						sscanf_s(data.c_str(), "%*s %*s %*d %s", file_name, sizeof file_name);

						if(!strlen(file_name))
						{
							addonDebug("NULL screenshot name");
						}

						gScreen->Get(std::string(file_name));
						gSocket->Send(formatString() << "TCPQUERY" << " " << "CLIENT_CALL" << " " << 1003 << " " << file_name);

						break;
					}
				}
			}
		}

		Sleep(1);
	}
	while(true);

	return true;
}
