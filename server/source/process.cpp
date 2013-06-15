#pragma once



#include "process.h"



extern amxMutex *gMutex;
extern amxPool *gPool;
extern amxSocket *gSocket;

extern bool gInit;
extern logprintf_t logprintf;

extern std::queue<amxKey> amxKeyQueue;
extern std::queue<sockStruct> recvQueue;





#ifdef WIN32
	DWORD process_thread(void *lpParam)
#else
	void *process_thread(void *lpParam)
#endif
{
	sockStruct data;

	char tcpquery[32];
	char call[32];
	char input[32768];
	int call_index;

	memset(tcpquery, NULL, sizeof tcpquery);
	memset(call, NULL, sizeof call);
	memset(input, NULL, sizeof input);

	gSocket->MaxClients(atoi(gPool->Get(-1, "maxclients").c_str()));
	gSocket->Bind(gPool->Get(-1, "ip"));
	gSocket->Listen(atoi(gPool->Get(-1, "port").c_str()));

	while(gInit)
	{
		if(!recvQueue.empty())
		{
			for(unsigned int i = 0; i < recvQueue.size(); i++)
			{
				gMutex->Lock();
				data = recvQueue.front();
				recvQueue.pop();
				gMutex->unLock();

				memset(tcpquery, NULL, sizeof tcpquery);
				memset(call, NULL, sizeof call);
				memset(input, NULL, sizeof input);

				sscanf_s(data.data.c_str(), "%s>%s>%d>%s", tcpquery, call, &call_index, input);

				logprintf("Data: %s>%s>%i>%s", tcpquery, call, call_index, input);

				if(strcmp(tcpquery, "TCPQUERY") || strcmp(call, "CLIENT_CALL"))
				{
					logprintf("Invalid query sent from %i", data.clientid);

					continue;
				}

				switch(call_index)
				{
				case 1000:
					{
						int serial;
						int serial_key;
						int flags_key;
						int flags;
						char name[25];

						sscanf_s(input, "%d>%d>%s>%d>%d", &serial, &serial_key, name, &flags_key, &flags);

						if((serial_key != (serial + (flags ^ 0x2296666))) || (flags_key != ((serial | flags) & 0x28F39)))
						{
							logprintf("Invalid auth key sent from %i", data.clientid);

							continue;
						}

						serial += flags;

						gPool->Set(data.clientid, "serial", formatString() << serial);
					}
					break;
				case 1001:
					{
						int dll;
						int reason;
						int reserved;

						sscanf_s(input, "%d_%d_%d", &dll, &reason, &reserved);

						logprintf("%i module %i unloaded (Reason: %i)", data.clientid, dll, reason);
					}
					break;
				case 1234:
					{
						amxKey amxKeyData;

						amxKeyData.clientID = data.clientid;
						amxKeyData.keyID = atoi(input);

						gMutex->Lock();
						amxKeyQueue.push(amxKeyData);
						gMutex->unLock();
					}
					break;
				}
			}
		}

		SLEEP(1);
	}

	#ifdef WIN32
		return true;
	#endif
}