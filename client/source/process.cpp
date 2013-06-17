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

	this->processHandle = gThread->Start((LPTHREAD_START_ROUTINE)process_thread, (void *)GetTickCount());
}



addonProcess::~addonProcess()
{
	addonDebug("Process deconstructor called");

	gThread->Stop(this->processHandle);
}



DWORD process_thread(void *lpParam)
{
	addonDebug("Thread 'process_thread' successfuly started");

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

						if(rec_serial == serial)
						{
							addonDebug("Serials matches");
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

					case 1004: // TCPQUERY SERVER_CALL 1004 %i1 %i2   ---   Gets client value from address | %i1 - address, %i2 - var type
					{
						int input;
						int type;

						sscanf_s(data.c_str(), "%*s %*s %*d %d %d", &input, &type);

						switch(type)
						{
							case 0:
							{
								// int

								break;
							}

							case 1:
							{
								// float
								float ret;

								if(input == 0x00863984)
									memcpy(&ret, (void *)input, sizeof(float));
								else
									ret = *(float *)input;

								gSocket->Send(formatString() << "TCPQUERY" << " " << "CLIENT_CALL" << " " << 1004 << " " << input << " " << ret);

								break;
							}

							case 2:
							{
								// string

								break;
							}

							case 3:
							{
								// uint8_t

								gSocket->Send(formatString() << "TCPQUERY" << " " << "CLIENT_CALL" << " " << 1004 << " " << input << " " << (int)*(uint8_t *)input);

								break;
							}

							case 4:
							{
								// uint32_t

								gSocket->Send(formatString() << "TCPQUERY" << " " << "CLIENT_CALL" << " " << 1004 << " " << input << " " << *(uint32_t *)input);

								break;
							}
						}

						break;
					}

					case 1005: // TCPQUERY SERVER_CALL 1005 %i1 %i2 %s1   ---   Sets client value (ptr = address) | %i1 - needle address, %i2 - var type, %s1 - needle data
					{
						int input;
						int type;

						sscanf_s(data.c_str(), "%*s %*s %*d %d %d", &input, &type);

						switch(type)
						{
							case 0:
							{
								// int
								int value;

								sscanf_s(data.c_str(), "%*s %*s %*d %*d %*d %d", &value);

								break;
							}

							case 1:
							{
								// float
								float value;

								sscanf_s(data.c_str(), "%*s %*s %*d %*d %*d %f", &value);
								*(float *)input = value;

								break;
							}

							case 2:
							{
								// string
								char value[256];

								sscanf_s(data.c_str(), "%*s %*s %*d %*d %*d %s", value, sizeof value);

								break;
							}

							case 3:
							{
								// uint8_t
								int value;

								sscanf_s(data.c_str(), "%*s %*s %*d %*d %*d %d", &value);
								*(uint8_t *)input = (uint8_t)value;

								break;
							}
						}

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
