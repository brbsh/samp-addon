#pragma once



#include "process.h"



extern amxMutex *gMutex;
extern amxPool *gPool;
extern amxSocket *gSocket;

extern logprintf_t logprintf;


extern std::queue<amxKey> amxKeyQueue;
extern std::queue<amxScreenshot> amxScreenshotQueue;
extern std::queue<processStruct> recvQueue;





#ifdef WIN32
	DWORD process_thread(void *lpParam)
#else
	void *process_thread(void *lpParam)
#endif
{
	processStruct input;

	int call_index;

	do
	{
		if(!recvQueue.empty())
		{
			for(unsigned int i = 0; i < recvQueue.size(); i++)
			{
				gMutex->Lock();
				input = recvQueue.front();
				recvQueue.pop();
				gMutex->unLock();

				sscanf_s(input.data.c_str(), "%*s %*s %d", &call_index);

				if(!gPool->clientPool[input.clientID].auth && (call_index != 1000))
				{
					logprintf("Invalid query from %i", input.clientID);

					gSocket->KickClient(input.clientID);

					continue;
				}

				switch(call_index)
				{
					case 1000: // TCPQUERY CLIENT_CALL 1000 %i1 %i2 %s1 %i3 %i4   ---   Auth key from client | %i1 - Client's HDD serial, %i2 - HMAC of %i1, %s1 - Client name, %i3 - HMAC of %i4, %i4 - HDD flags
					{
						cliPool cPool;

						int serial;
						int serial_key;
						int flags_key;
						int flags;
						char name[25];

						sscanf_s(input.data.c_str(), "%*s %*s %*d %d %d %s %d %d", &serial, &serial_key, name, sizeof name, &flags_key, &flags);

						if((serial_key != (serial + (flags ^ 0x2296666))) || (flags_key != ((serial | flags) & 0x28F39)))
						{
							logprintf("Invalid auth key sent from %i", input.clientID);

							continue;
						}

						serial += flags;

						cPool = gPool->clientPool[input.clientID];
						cPool.serial = serial;
						cPool.auth = true;
						gPool->clientPool[input.clientID] = cPool;

						gSocket->Send(input.clientID, formatString() << "TCPQUERY" << " " << "SERVER_CALL" << " " << 1000 << " " << serial);

						break;
					}

					case 1001: // TCPQUERY CLIENT_CALL 1001 %i1 %i2 %i3   ---   Sends addon module detach info | %i1 - DLL address, %i2 - Detach reason, %i3 - Reserved var
					{
						int dll;
						int reason;
						int reserved;

						sscanf_s(input.data.c_str(), "%*s %*s %*d %d %d %d", &dll, &reason, &reserved);

						logprintf("%i module %i unloaded (Reason: %i)", input.clientID, dll, reason);

						break;
					}

					case 1002: // TCPQUERY CLIENT_CALL 1002 %s1    ---   Sends async key data | %s1 - Keys array
					{
						amxKey amxKeyData;

						input.data.erase(0, (input.data.find("1002 ") + 5));

						amxKeyData.clientID = input.clientID;
						amxKeyData.numcells = input.data.length();
						amxKeyData.arr = (cell *)malloc(sizeof(cell) * (amxKeyData.numcells + 1));

						memcpy(amxKeyData.arr, input.data.data(), amxKeyData.numcells);
						
						gMutex->Lock();
						amxKeyQueue.push(amxKeyData);
						gMutex->unLock();

						delete[] amxKeyData.arr;

						break;
					}

					case 1003: // TCPQUERY CLIENT_CALL 1003 %s1   ---   Screenshot was taken | %s1 - Screenshot name
					{
						amxScreenshot amxScreenshotData;

						input.data.erase(0, (input.data.find("1003 ") + 5));

						amxScreenshotData.clientID = input.clientID;
						amxScreenshotData.name = input.data;

						gMutex->Lock();
						amxScreenshotQueue.push(amxScreenshotData);
						gMutex->unLock();

						break;
					}
				}
			}
		}

		SLEEP(1);
	}
	while((gSocket->socketInfo.active) && (gPool->pluginInit));

	#ifdef WIN32
		return true;
	#endif
}