#pragma once



#include "process.h"





addonProcess *gProcess;


extern addonData *gData;
extern addonTransfer *gTransfer;
extern addonSocket *gSocket;
extern addonScreen *gScreen;

extern std::queue<std::string> recvQueue;





addonProcess::addonProcess()
{
	addonDebug("Process constructor called");

	boost::mutex::scoped_lock lock(this->Mutex);
	this->threadActive = true;
	lock.unlock();

	boost::thread process(boost::bind(&addonProcess::Thread));
}



addonProcess::~addonProcess()
{
	addonDebug("Process deconstructor called");

	boost::mutex::scoped_lock lock(this->Mutex);
	this->threadActive = false;
	lock.unlock();
}



void addonProcess::Thread()
{
	addonDebug("Thread addonProcess::Thread() successfuly started");

	int call_index;

	boost::mutex pMutex;
	std::string data;

	do
	{
		if(!recvQueue.empty())
		{
			for(unsigned int i = 0; i < recvQueue.size(); i++)
			{
				boost::mutex::scoped_lock lock(pMutex);
				data = recvQueue.front();
				recvQueue.pop();
				lock.unlock();

				sscanf_s(data.c_str(), "%*s %*s %i", &call_index);

				switch(call_index)
				{
					case 1000: // TCPQUERY SERVER_CALL 1000 %i1   ---   Response from initial query | %i1 - your HDD serial
					{
						int rec_serial;

						sscanf_s(data.c_str(), "%*s %*s %*i %i", &rec_serial);

						char sysdrive[5];
						WCHAR wsysdrive[5];

						DWORD serial = NULL;
						DWORD flags = NULL;

						strcpy_s(sysdrive, getenv("SystemDrive"));
						strcat_s(sysdrive, "\\");

						MultiByteToWideChar(NULL, NULL, sysdrive, sizeof sysdrive, wsysdrive, sizeof wsysdrive);
						GetVolumeInformationW(wsysdrive, NULL, NULL, &serial, NULL, &flags, NULL, NULL);

						serial += flags;

						if(rec_serial != serial)
						{
							addonDebug("Invalid server answer, terminating");

							boost::mutex::scoped_lock lock(pMutex);
							delete gData;
							lock.unlock();
						}

						break;
					}

					case 1001: // TCPQUERY SERVER_CALL 1001 %i1   ---   Calls when error occured while connecting to TCP server | %i1 - error code
					{
						int error_code;

						sscanf_s(data.c_str(), "%*s %*s %*i %i", &error_code);

						if(!error_code)
						{
							addonDebug("TCP server is full");

							boost::mutex::scoped_lock lock(pMutex);
							delete gData;
							lock.unlock();
						}

						break;
					}

					//case 1002:
					case 1003: // TCPQUERY SERVER_CALL 1003 %s1   ---   Screenshot take request from server | %s1 - local file name
					{
						char file_name[256];

						sscanf_s(data.c_str(), "%*s %*s %*i %s", file_name, sizeof file_name);

						if(!strlen(file_name))
						{
							addonDebug("NULL screenshot name");
						}

						boost::mutex::scoped_lock lock(pMutex);
						gScreen->Get(file_name);
						lock.unlock();

						gSocket->Send(formatString() << "TCPQUERY CLIENT_CALL " << 1003 << " " << file_name);

						break;
					}

					case 2000: // TCPQUERY SERVER_CALL 2000 %s1 %i1   ---   Remote file transfer | %s1 - File name, %i1 - file length
					{
						char file_name[256];
						int length = NULL;

						sscanf_s(data.c_str(), "%*s %*s %*i %s %i", file_name, sizeof file_name, &length);

						if(!strcmp(file_name, "START"))
						{
							addonDebug("File transfering from server started");

							break;
						}

						gSocket->Socket->write_some(boost::asio::buffer("TCPQUERY CLIENT_CALL 2000 START", 32));

						boost::mutex::scoped_lock lock(pMutex);
						gData->Transfer.Active = true;
						lock.unlock();

						boost::thread receive(boost::bind(&addonTransfer::ReceiveThread, file_name, length));

						break;
					}

					case 2001: // TCPQUERY SERVER_CALL 2001 %s1   ---   Local file transfer | %s1 - File name
					{
						char file_name[256];

						sscanf_s(data.c_str(), "%*s %*s %*i %s", file_name, sizeof file_name);

						if(!strcmp(file_name, "START"))
						{
							addonDebug("File transfering to server started");

							break;
						}

						boost::mutex::scoped_lock lock(pMutex);
						gData->Transfer.Active = true;
						lock.unlock();

						boost::thread send(boost::bind(&addonTransfer::SendThread, file_name));

						break;
					}
				}

				boost::this_thread::sleep(boost::posix_time::milliseconds(1));
			}
		}

		boost::this_thread::sleep(boost::posix_time::milliseconds(1));
	}
	while(gProcess->threadActive);
}
