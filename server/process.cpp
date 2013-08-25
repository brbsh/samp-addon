#pragma once



#include "process.h"





amxProcess *gProcess;


extern logprintf_t logprintf;

extern amxHash *gHash;
extern amxPool *gPool;
extern amxSocket *gSocket;

extern std::queue<amxDisconnect> amxDisconnectQueue;
extern std::queue<amxKey> amxKeyQueue;
extern std::queue<amxScreenshot> amxScreenshotQueue;
extern std::queue<processStruct> recvQueue;





void amxProcess::Thread()
{
	int call_index;

	processStruct input;

	boost::mutex pMutex;

	addonDebug("Thread amxProcess::Thread() successfuly started");

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

				if((call_index < 1000) || (!gPool->clientPool.find(input.clientID)->second.Client.Auth && (call_index != 1000)))
				{
					addonDebug("Invalid query from %i", input.clientID);

					gSocket->KickClient(input.clientID);

					continue;
				}

				switch(call_index)
				{
					case 1000: // TCPQUERY CLIENT_CALL 1000 %i1 %i2 %s1 %i3 %i4   ---   Auth key from client | %i1 - Client's HDD serial, %i2 - HMAC of %i1, %s1 - Client name, %i3 - HMAC of %i4, %i4 - HDD flags
					{
						cliPool cPool;

						int serial = NULL;
						int flags = NULL;
						int name_hash = NULL;
						int key = NULL;

						#if defined WIN32
							sscanf_s(input.data.c_str(), "%*s %*s %*i %i %i %i %i", &serial, &flags, &name_hash, &key);
						#else
							sscanf(input.data.c_str(), "%*s %*s %*i %i %i %i %i", &serial, &flags, &name_hash, &key);
						#endif

						if(!serial || !flags || !name_hash || (key != (serial ^ flags | name_hash ^ gSocket->Port)))
						{
							addonDebug("Invalid auth key sent from %i", input.clientID);

							gSocket->KickClient(input.clientID);

							continue;
						}

						serial += flags;

						boost::mutex::scoped_lock lock(pMutex);
						cPool = gPool->clientPool.find(input.clientID)->second;

						cPool.Client.Serial = serial;
						cPool.Client.Auth = true;

						gPool->clientPool[input.clientID] = cPool;
						lock.unlock();

						gSocket->Send(input.clientID, formatString() << "TCPQUERY" << " " << "SERVER_CALL" << " " << 1000 << " " << serial);
						
						break;
					}

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

						break;
					}

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

						break;
					}

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

						break;
					}

					case 2000:
					{
						char code[16];

						#if defined WIN32
							sscanf_s(input.data.c_str(), "%*s %*s %*i %s", code, sizeof code);
						#else
							sscanf(input.data.c_str(), "%*s %*s %*i %s", code);
						#endif

						if(strcmp(code, "START"))
							addonDebug("File transfering to client %i started", input.clientID);
						else
							addonDebug("Invalid start header sent from %i", input.clientID);
						
						break;
					}

					case 2001: // TCPQUERY CLIENT_CALL 2001 %s1 %i1   ---   Transfer file data | %s1 - Remote file name, %i1 - Remote file length
					{
						char file[256];
						int length = NULL;

						#if defined WIN32
							sscanf_s(input.data.c_str(), "%*s %*s %*i %s %i", file, sizeof file, &length);
						#else
							sscanf(input.data.c_str(), "%*s %*s %*i %s %i", file, &length);
						#endif

						if((!strcmp(file, "END") || !strcmp(file, "READY")) && !length)
						{
							logprintf("Unexpected value passed!");

							continue;
						}

						cliPool cPool;

						boost::mutex::scoped_lock lock(pMutex);
						cPool = gPool->clientPool.find(input.clientID)->second;

						cPool.Transfer.Active = true;

						gPool->clientPool[input.clientID] = cPool;
						lock.unlock();

						cPool.socketid->write_some(boost::asio::buffer("TCPQUERY SERVER_CALL 2001 START", 32));

						boost::thread receive(boost::bind(&amxTransfer::ReceiveThread, input.clientID, cPool.Transfer.file, length));
					
						break;
					}
				}

				boost::this_thread::sleep(boost::posix_time::milliseconds(1));
			}
		}

		boost::this_thread::sleep(boost::posix_time::milliseconds(1));
	}
	while(/*(gSocket->socketInfo.active) && */(gPool->pluginInit));
}