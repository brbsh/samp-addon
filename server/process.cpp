#pragma once



#include "process.h"





amxProcess *gProcess;


extern logprintf_t logprintf;

extern amxPool *gPool;
extern amxSocket *gSocket;

extern std::queue<amxDisconnect> amxDisconnectQueue;
extern std::queue<amxKey> amxKeyQueue;
extern std::queue<amxScreenshot> amxScreenshotQueue;
extern std::queue<processStruct> recvQueue;





void amxProcess::Thread(AMX *amx)
{
	addonDebug("Thread amxProcess::Thread(%i) successfuly started", (int)amx);

	boost::mutex pMutex;

	int call_index;
	processStruct input;

	do
	{
		if(!recvQueue.empty())
		{
			for(unsigned int i = 0; i < recvQueue.size(); i++)
			{
				boost::mutex::scoped_lock lock(pMutex);
				input = recvQueue.front();
				recvQueue.pop();
				lock.unlock();

				#if defined WIN32
					sscanf_s(input.data.c_str(), "%*s %*s %i", &call_index);
				#else
					sscanf(input.data.c_str(), "%*s %*s %i", &call_index);
				#endif

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

						#if defined WIN32
							sscanf_s(input.data.c_str(), "%*s %*s %*i %i %i %s %i %i", &serial, &serial_key, name, sizeof name, &flags_key, &flags);
						#else
							sscanf(input.data.c_str(), "%*s %*s %*i %i %i %s %i %i", &serial, &serial_key, name, &flags_key, &flags);
						#endif

						if((serial_key != (serial + (flags ^ 0x2296666))) || (flags_key != ((serial | flags) & 0x28F39)))
						{
							logprintf("samp-addon: Invalid auth key sent from %i", input.clientID);

							continue;
						}

						serial += flags;

						boost::mutex::scoped_lock lock(pMutex);
						cPool = gPool->clientPool[input.clientID];

						cPool.serial = serial;
						cPool.auth = true;

						gPool->clientPool[input.clientID] = cPool;
						lock.unlock();

						gSocket->Send(input.clientID, formatString() << "TCPQUERY" << " " << "SERVER_CALL" << " " << 1000 << " " << serial);
					}
					break;
					case 1001: // TCPQUERY CLIENT_CALL 1001 %i1   ---   Sends addon module detach info | %i1 - Reason
					{
						amxDisconnect pushme;

						int reason;

						#if defined WIN32
							sscanf_s(input.data.c_str(), "%*s %*s %*i %i", &reason);
						#else
							sscanf(input.data.c_str(), "%*s %*s %*i %i", &reason);
						#endif

						pushme.clientID = input.clientID;

						boost::mutex::scoped_lock lock(pMutex);
						amxDisconnectQueue.push(pushme);
						lock.unlock();
					}
					break;
					case 1002: // TCPQUERY CLIENT_CALL 1002 %s1    ---   Sends async key data | %s1 - Keys array
					{
						amxKey amxKeyData;

						#if defined WIN32
							sscanf_s(input.data.c_str(), "%*s %*s %*i %s", amxKeyData.keys, sizeof amxKeyData.keys);
						#else
							sscanf(input.data.c_str(), "%*s %*s %*i %s", amxKeyData.keys);
						#endif

						amxKeyData.clientID = input.clientID;
						
						boost::mutex::scoped_lock lock(pMutex);
						amxKeyQueue.push(amxKeyData);
						lock.unlock();
					}
					break;
					case 1003: // TCPQUERY CLIENT_CALL 1003 %s1   ---   Screenshot was taken | %s1 - Screenshot name
					{
						amxScreenshot amxScreenshotData;

						#if defined WIN32
							sscanf_s(input.data.c_str(), "%*s %*s %*i %s", amxScreenshotData.name, sizeof amxScreenshotData.name);
						#else
							sscanf(input.data.c_str(), "%*s %*s %*i %s", amxScreenshotData.name);
						#endif

						amxScreenshotData.clientID = input.clientID;

						boost::mutex::scoped_lock lock(pMutex);
						amxScreenshotQueue.push(amxScreenshotData);
						lock.unlock();
					}
					break;
					case 2001: // TCPQUERY CLIENT_CALL 2001 %s1 %i1   ---   Transfer file data | %s1 - Remote file name, %i1 - Remote file length
					{
						char check[10];
						int length;

						#if defined WIN32
							sscanf_s(input.data.c_str(), "%*s %*s %*i %s %i", check, sizeof check, &length);
						#else
							sscanf(input.data.c_str(), "%*s %*s %*i %s %i", check, &length);
						#endif

						if(!strcmp(check, "END") || !strcmp(check, "READY"))
						{
							logprintf("Unexpected value passed!");

							continue;
						}

						transPool pool;
						sockPool sPool;

						boost::mutex::scoped_lock lock(pMutex);
						pool = gPool->transferPool[input.clientID];
						sPool = gPool->socketPool[input.clientID];

						pool.file_length = length;
						sPool.transfer = true;

						gPool->transferPool[input.clientID] = pool;
						gPool->socketPool[input.clientID] = sPool;
						lock.unlock();

						boost::thread receive(boost::bind(&amxTransfer::ReceiveThread, input.clientID));
					}
					break;
				}

				boost::this_thread::sleep(boost::posix_time::milliseconds(1));
			}
		}

		boost::this_thread::sleep(boost::posix_time::milliseconds(1));
	}
	while(/*(gSocket->socketInfo.active) && */(gPool->pluginInit));
}