#pragma once



#include "process.h"



extern amxMutex *gMutex;
extern amxPool *gPool;
extern amxSocket *gSocket;
extern amxThread *gThread;

extern logprintf_t logprintf;


extern std::queue<amxDisconnect> amxDisconnectQueue;
extern std::queue<amxKey> amxKeyQueue;
extern std::queue<amxScreenshot> amxScreenshotQueue;
extern std::queue<processStruct> recvQueue;





#ifdef WIN32
	DWORD amxProcess::Thread(void *lpParam)
#else
	void *amxProcess::Thread(void *lpParam)
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
					logprintf("samp-addon: Invalid query from %i", input.clientID);

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
							logprintf("samp-addon: Invalid auth key sent from %i", input.clientID);

							continue;
						}

						serial += flags;

						gMutex->Lock();
						cPool = gPool->clientPool[input.clientID];
						gMutex->unLock();

						cPool.serial = serial;
						cPool.auth = true;

						gMutex->Lock();
						gPool->clientPool[input.clientID] = cPool;
						gMutex->unLock();

						gSocket->Send(input.clientID, formatString() << "TCPQUERY" << " " << "SERVER_CALL" << " " << 1000 << " " << serial);

						break;
					}

					case 1001: // TCPQUERY CLIENT_CALL 1001 %i1   ---   Sends addon module detach info | %i1 - Reason
					{
						amxDisconnect pushme;

						int reason;

						sscanf_s(input.data.c_str(), "%*s %*s %*d %d", &reason);
						pushme.clientID = input.clientID;

						gMutex->Lock();
						amxDisconnectQueue.push(pushme);
						gMutex->unLock();

						break;
					}

					case 1002: // TCPQUERY CLIENT_CALL 1002 %s1    ---   Sends async key data | %s1 - Keys array
					{
						amxKey amxKeyData;

						sscanf_s(input.data.c_str(), "%*s %*s %*d %s", amxKeyData.keys, sizeof amxKeyData.keys);
						amxKeyData.clientID = input.clientID;
						
						gMutex->Lock();
						amxKeyQueue.push(amxKeyData);
						gMutex->unLock();

						break;
					}

					case 1003: // TCPQUERY CLIENT_CALL 1003 %s1   ---   Screenshot was taken | %s1 - Screenshot name
					{
						amxScreenshot amxScreenshotData;

						sscanf_s(input.data.c_str(), "%*s %*s %*d %s", amxScreenshotData.name, sizeof amxScreenshotData.name);
						amxScreenshotData.clientID = input.clientID;

						gMutex->Lock();
						amxScreenshotQueue.push(amxScreenshotData);
						gMutex->unLock();

						break;
					}

					case 2001: // TCPQUERY CLIENT_CALL 2001 %s1 %i1   ---   Transfer file data | %s1 - Remote file name, %i1 - Remote file length
					{
						char check[10];
						int length;

						sscanf_s(input.data.c_str(), "%*s %*s %*d %s %d", check, sizeof check, &length);

						if(!strcmp(check, "END") || !strcmp(check, "READY"))
						{
							logprintf("Unexpected value passed!");

							break;
						}

						transPool pool;

						gMutex->Lock();
						pool = gPool->transferPool[input.clientID];
						pool.file_length = length;
						gPool->transferPool[input.clientID] = pool;
						gMutex->unLock();

						sockPool sPool;

						gMutex->Lock();
						sPool = gPool->socketPool[input.clientID];
						sPool.transfer = true;
						gPool->socketPool[input.clientID] = sPool;
						gMutex->unLock();

						gThread->Start(amxTransfer::ReceiveThread, (void *)input.clientID);

						break;
					}
				}
			}
		}

		SLEEP(10);
	}
	while((gSocket->socketInfo.active) && (gPool->pluginInit));

	#ifdef WIN32
		return true;
	#endif
}