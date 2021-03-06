#pragma once



#include "server.hpp"





boost::shared_ptr<amxCore> gCore;


extern boost::shared_ptr<amxDebug> gDebug;
extern boost::shared_ptr<amxPool> gPool;
extern boost::shared_ptr<amxSocket> gSocket;
extern boost::shared_ptr<amxUpdater> gUpdater;





amxCore::amxCore()
{
	gDebug->Log("Core constructor called");

	gPool = boost::shared_ptr<amxPool>(new amxPool());
	//gUpdater = boost::shared_ptr<amxUpdater>(new amxUpdater());

	threadInstance = boost::shared_ptr<boost::thread>(new boost::thread(boost::bind(&amxCore::Thread)));
}



amxCore::~amxCore()
{
	gDebug->Log("Core destructor called");

	gPool.reset();
	//gUpdater.reset();

	threadInstance->interrupt();
}



void amxCore::processFunc(unsigned int maxclients)
{
	std::vector<std::string> data;
	unsigned short iter;
	int rescode;

	for(unsigned int clientid = 0; clientid < maxclients; clientid++)
	{
		if(!gSocket->IsClientConnected(clientid))
			continue;

		amxAsyncSession *session = gPool->getClientSession(clientid);
		data.clear();
		iter = NULL;

		if(session->pool().connstate != 2)
			continue;

		boost::unique_lock<boost::mutex> lockit(session->pool().pqMutex);

		if(session->pool().pendingQueue.empty())
		{
			lockit.unlock();
			continue;
		}

		data = session->pool().pendingQueue.front();
		session->pool().pendingQueue.pop();

		lockit.unlock();

		if(data.at(0).compare("CMDRESPONSE") != std::string::npos) // response from client on server query success/fail
		{
			if(!amxString::isDecimial(data.at(1).c_str(), data.at(1).length()))
			{
				gDebug->Log("Client %i sent crafted response (What: Response code is non-numeric)", clientid);
				gSocket->KickClient(clientid, "amxCore::processFunc: Response code NaN");

				continue;
			}

			rescode = boost::lexical_cast<int>(data.at(1));

			if(rescode != session->pool().cmdresponse_state)
			{
				gDebug->Log("Client %i sent invalid response");
				gSocket->KickClient(clientid, "amxCore::processFunc: Invalid response");

				continue;
			}

			session->pool().cmdresponse_state = NULL;

			switch(rescode)
			{
				case ADDON_CMD_QUERY_SCREENSHOT: // CMDRESPONSE|ADDON_CMD_QUERY_SCREENSHOT|*ERROR STATUS*|*FILE PATH*  == Screenshot query
				{
					if(data.size() != 4)
					{
						// invalid arg count
					}

					/*if(boost::regex_search(data.at(3), boost::regex("[.:%]{1,2}[/\\]+")))
					{
						// found dir hack keywords

						continue;
					}*/

					amxCore::amxPush toAMX;
					amxPool::svrData toData;

					toAMX.clientid = clientid;

					if(data.at(2).compare("OK") == std::string::npos)
					{
						// error occured, push to amx
						toData.reset();
						toData.string.assign(data.at(3));
						toAMX.args.push_back(toData);

						toData.reset();
						toData.string.append(data.at(2));
						toAMX.args.push_back(toData);

						pushToPT(ADDON_CALLBACK_OCSE, toAMX); // Addon_OnClientScreenshotError(clientid, data.at(3), data.at(2));

						continue;
					}

					toData.reset();
					toData.string.assign(data.at(3));
					toAMX.args.push_back(toData);

					pushToPT(ADDON_CALLBACK_OCST, toAMX); // Addon_OnClientScreenshotTaken(clientid, data.at(3));
				}
				break;

				case ADDON_CMD_QUERY_TRF: // CMDRESPONSE|ADDON_CMD_QUERY_TRF|*remote file name*|*file size*|*file CRC*
				{
					if(data.size() != 5)
					{
						// invalid arg count
					}

					std::size_t remote_file_size = boost::lexical_cast<std::size_t>(data.at(3));
					int remote_file_crc = boost::lexical_cast<int>(data.at(4));

					session->pool().file_t = true;
					session->pool().fileT = new amxTransfer(false, clientid, session->pool().file_tmp_name, data.at(2), remote_file_size, remote_file_crc);
				}
				break;
			}
		}
		else if(data.at(0).compare("CMDDATASENT") != std::string::npos) // data from client (AFK status, keys pressed etc.)
		{
			
		}
		else
		{
			gDebug->Log("Unknown query from client %i", clientid);

			continue;
		}

		boost::this_thread::yield();
	}
}



void amxCore::Thread()
{
	assert(gCore->getThreadInstance()->get_id() == boost::this_thread::get_id());

	gDebug->Log("Thread amxCore::Thread() successfuly started");

	while(!gPool->getPluginStatus())
		boost::this_thread::sleep(boost::posix_time::seconds(1)); //boost::this_thread::sleep_for(boost::chrono::seconds(1));

	amxPool::svrData data = gPool->getServerVar("ip");
	std::string ip = data.string;

	data.reset();
	data = gPool->getServerVar("port");
	unsigned short port = data.integer;

	data.reset();
	data = gPool->getServerVar("maxclients");
	unsigned int maxclients = data.integer;

	gSocket = boost::shared_ptr<amxSocket>(new amxSocket(ip, port, maxclients));

	do
	{
		boost::this_thread::disable_interruption di;

		gCore->processFunc(maxclients);

		boost::this_thread::restore_interruption re(di);
		boost::this_thread::sleep(boost::posix_time::milliseconds(10)); //boost::this_thread::sleep_for(boost::chrono::milliseconds(10));
	}
	while(gPool->getPluginStatus());

	gSocket.reset();
}