#pragma once



#include "client.hpp"





boost::shared_ptr<addonCore> gCore;


extern boost::shared_ptr<addonD3Device> gD3Device;
extern boost::shared_ptr<addonDebug> gDebug;
extern boost::shared_ptr<addonFunctions> gFunctions;
extern boost::shared_ptr<addonLoader> gLoader;
extern boost::shared_ptr<addonPool> gPool;
extern boost::shared_ptr<addonSocket> gSocket;
extern boost::shared_ptr<addonTransfer> gTransfer;





addonCore::addonCore()
{
	gFunctions = boost::shared_ptr<addonFunctions>(new addonFunctions());
	gPool = boost::shared_ptr<addonPool>(new addonPool());
	gSocket = boost::shared_ptr<addonSocket>(new addonSocket());

	gDebug->traceLastFunction("addonCore::addonCore() at 0x%x", &gCore);
	gDebug->Log("Core constructor called");

	threadInstance = boost::shared_ptr<boost::thread>(new boost::thread(boost::bind(&addonCore::Thread)));
}



addonCore::~addonCore()
{
	gDebug->traceLastFunction("addonCore::~addonCore() at 0x?????");
	gDebug->Log("Core destructor called");

	threadInstance->interrupt();
	threadInstance.reset();

	gLoader.reset();
	gSocket.reset();
	gPool.reset();
	gFunctions.reset();
}



void addonCore::queueIN(addonSocket *instance)
{
	char buffer[2048];
	std::size_t bytes_rx;

	try
	{
		memset(buffer, NULL, sizeof(buffer));
		bytes_rx = instance->getSocket().read_some(boost::asio::buffer(buffer));
	}
	catch(boost::system::system_error& error)
	{
		gDebug->Log("Cannot read from socket (What: %s)", error.what());
		gSocket->disconnect();

		return;
	}

	if((bytes_rx > 0) && (bytes_rx <= sizeof(buffer)))
	{
		gDebug->Log("Got '%s' (length: %i) from server", buffer, bytes_rx);

		std::istringstream input(buffer);
		std::string output;
		std::vector<std::string> args;

		int packet_crc;
		int remote_packet_crc;

		std::getline(input, output, '|');
		remote_packet_crc = boost::lexical_cast<int>(output);

		std::getline(input, output, '\0');
		packet_crc = addonHash::crc32(output, output.length());

		if(packet_crc != remote_packet_crc)
		{
			gDebug->Log("Server sent bad packet: CRC not passed [L:%i != R:%i]", packet_crc, remote_packet_crc);
			gSocket->disconnect();

			return;
		}

		boost::split(args, output, boost::is_any_of("|"));

		try_lock_mutex:

		if(pqMutex.try_lock())
		{
			pendingQueue.push(args);
			pqMutex.unlock();
		}
		else
		{
			gDebug->Log("Cannot lock pending queue mutex, continuing anyway");
			boost::this_thread::yield();
			goto try_lock_mutex;
		}
	}
	else
	{
		gDebug->Log("Invalid length (%i) returned by socket read function", bytes_rx);

		gSocket->disconnect();
	}
}



void addonCore::processFunc()
{
	while(true)
	{
		pqMutex.lock();

		if(pendingQueue.empty())
		{
			pqMutex.unlock();

			break;
		}

		if(gTransfer->isTransfering())
		{
			boost::this_thread::sleep(boost::posix_time::milliseconds(50));

			continue;
		}

		std::vector<std::string> data = pendingQueue.front();
		pqMutex.unlock();

		if(data.at(0).compare("CMDQUERY") != std::string::npos)
		{
			unsigned int code = boost::lexical_cast<unsigned int>(data.at(1));

			switch(code)
			{
				case ADDON_CMD_QUERY_SCREENSHOT: // CMDQUERY|ADDON_CMD_QUERY_SCREENSHOT|*filename*
				{
					if(data.size() != 3)
					{
						//error
					}

					/*if(boost::regex_search(data.at(2), boost::regex("[.:%]{1,2}[/\\]+")))
					{
						// found dir hack
					}*/

					if(gD3Device->Screenshot(data.at(2)))
						gSocket->writeTo(boost::str(boost::format("CMDRESPONSE|%1%|OK|%2%") % ADDON_CMD_QUERY_SCREENSHOT % data.at(2)));
					else
						gSocket->writeTo(boost::str(boost::format("CMDRESPONSE|%1%|ERROR|%2%") % ADDON_CMD_QUERY_SCREENSHOT % data.at(2)));
				}
				break;

				case ADDON_CMD_QUERY_TLF: // CMDQUERY|ADDON_CMD_QUERY_TLF|*filename*|*file size*|*file CRC*
				{
					if(data.size() != 5)
					{
						// error
					}

					std::size_t remote_file_size = boost::lexical_cast<std::size_t>(data.at(3));
					int remote_file_crc = boost::lexical_cast<int>(data.at(4));

					/*if(boost::filesystem::exists(boost::filesystem::path(data.at(2))) && (remote_file_crc == addonHash::crc32_file(data.at(2))))
					{
						// file already exists
					}*/

					gTransfer->recvFile(data.at(2), remote_file_size, remote_file_crc);
				}
				break;

				case ADDON_CMD_QUERY_TRF: // CMDQUERY|ADDON_CMD_QUERY_TRF|*filename*
				{
					if(data.size() != 3)
					{
						// error
					}

					gTransfer->sendFile(data.at(2));
				}
				break;
			}
		}
		else
		{
			gDebug->Log("Invalid query from server");

			return;
		}

		try_lock_mutex:

		if(pqMutex.try_lock())
		{
			pendingQueue.pop();
			pqMutex.unlock();
		}
		else
		{
			gDebug->Log("Cannot lock pending queue mutex, continuing anyway");
			boost::this_thread::yield();
			goto try_lock_mutex;
		}

		boost::this_thread::yield();
	}
}



void addonCore::Thread()
{
	assert(gCore->getThreadInstance()->get_id() == boost::this_thread::get_id());

	gDebug->traceLastFunction("addonCore::Thread() at 0x%x", &addonCore::Thread);
	gDebug->Log("Started Core thread");

	boost::this_thread::sleep(boost::posix_time::seconds(5)); //boost::this_thread::sleep_for(boost::chrono::seconds(5));

	while(!gD3Device->getDevice(false)) // Wait until we create D3D device
		boost::this_thread::sleep(boost::posix_time::seconds(1)); //boost::this_thread::sleep_for(boost::chrono::seconds(1));

	gD3Device->renderText("Loading SAMP-Addon...", 10, 10, 255, 255, 255, 127);
	gD3Device->renderText("Scanning GTA dir for illegal files...", 10, 30, 255, 255, 255, 127);

	gLoader = boost::shared_ptr<addonLoader>(new addonLoader());

	gD3Device->clearRender();
	gD3Device->renderText("Loading SAMP-Addon... OK!", 10, 10, 255, 255, 255, 127);
	gD3Device->renderText("Scanning GTA dir for illegal files... OK!", 10, 30, 255, 255, 255, 127);

	std::string serial = gPool->getVar("serial");

	gD3Device->renderText(boost::str(boost::format("Your UID is: %1%") % serial), 300, 30, 255, 255, 255, 127);

	while(!GTA_PED_STATUS_ADDR) // PED context load flag
		boost::this_thread::sleep(boost::posix_time::milliseconds(10)); ///boost::this_thread::sleep_for(boost::chrono::milliseconds(15));

	gD3Device->clearRender();

	gFunctions->ToggleGrassRendering(true);
	gFunctions->ToggleMotionBlur(true);
	//gFunctions->ToggleHUD(false);
	gFunctions->ToggleSeaWays(true);

	while(true)
	{
		boost::this_thread::disable_interruption di;

		gCore->processFunc();

		boost::this_thread::restore_interruption re(di);
		boost::this_thread::sleep(boost::posix_time::milliseconds(25)); //boost::this_thread::sleep_for(boost::chrono::milliseconds(50));
	}
}