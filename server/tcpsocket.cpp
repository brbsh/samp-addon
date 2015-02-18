#pragma once



#include "server.hpp"





boost::shared_ptr<amxSocket> gSocket;


extern boost::shared_ptr<amxCore> gCore;
extern boost::shared_ptr<amxDebug> gDebug;
extern boost::shared_ptr<amxPool> gPool;





amxSocket::amxSocket(std::string ip, unsigned short port, unsigned int maxclients)
{
	gDebug->Log("Socket constructor called");

	unsigned short worker_count = /*boost::thread::hardware_concurrency()*/ 1;

	threadInstance = boost::shared_ptr<boost::thread>(new boost::thread(boost::bind(&amxSocket::acceptThread, ip, port, maxclients, worker_count)));
	deadlineThreadInstance = boost::shared_ptr<boost::thread>(new boost::thread(boost::bind(&amxSocket::deadlineThread, maxclients)));

	/*threadGroup = boost::shared_ptr<boost::thread_group>(new boost::thread_group());

	for(short i = 0; i < worker_count; i++)
		threadGroup->create_thread(boost::bind(&amxSocket::acceptThread, ip, port, maxclients, (i + 1)));*/
}



amxSocket::~amxSocket()
{
	gDebug->Log("Socket destructor called");

	threadInstance->interruption_requested();
	deadlineThreadInstance->interruption_requested();

	//threadGroup->interrupt_all();
}



bool amxSocket::IsClientConnected(unsigned int clientid)
{
	return gPool->hasOwnSession(clientid);
}



void amxSocket::KickClient(unsigned int clientid, std::string reason)
{
	if(!IsClientConnected(clientid))
		return;

	amxCore::amxPush toAMX;
	amxAsyncSession *sess = gPool->getClientSession(clientid);

	toAMX.clientid = clientid;
	toAMX.pushDataFirst.assign(sess->pool().ip.to_string());
	toAMX.pushDataSecond.assign(boost::str(boost::format("1488|%1%") % reason));

	gCore->pushToPT(ADDON_CALLBACK_OCD, toAMX);
	gDebug->Log("Client %i was kicked (Reason: %s)", clientid, reason.c_str());

	gPool->resetOwnSession(clientid);
}



void amxSocket::acceptThread(std::string ip, unsigned short port, unsigned int maxclients, unsigned short workerID)
{
	gDebug->Log("Thread amxSocket::acceptThread(worker #%i) successfuly started", workerID);

	boost::asio::io_service io_service;

	while(gPool->getPluginStatus())
	{
		amxCore::amxPush toAMX;

		try
		{
			amxAsyncServer server(io_service, ip, port, maxclients);

			toAMX.clientid = workerID;

			gCore->pushToPT(ADDON_CALLBACK_OTWS, toAMX); // Addon_OnTCPWorkerStarted(workerID);
			gDebug->Log("TCP worker #%i started on %s:%i with max connections: %i", workerID, ip.c_str(), port, maxclients);
			io_service.run(); // blocking thread, releasing when error occurs
		}
		catch(boost::system::system_error& err)
		{
			io_service.stop();
			io_service.reset();
			gDebug->Log("Error while processing TCP worker #%i execution (What: %s)", workerID, err.what());

			toAMX.clientid = workerID;
			toAMX.pushDataFirst.assign(err.what());

			gCore->pushToPT(ADDON_CALLBACK_OTWE, toAMX); // Addon_OnTCPWorkerError(workerID, err.what());
		}

		gDebug->Log("Restarting TCP worker #%i due to stop detected...", workerID);

		boost::this_thread::sleep(boost::posix_time::seconds(5)); // boost::this_thread::sleep_for(boost::chrono::seconds(5));
	}
}



void amxSocket::deadlineThread(unsigned int maxclients)
{
	gDebug->Log("Thread amxSocket::deadlineThread(maxclients = %i) sucessfully started", maxclients);

	while(gPool->getPluginStatus())
	{
		boost::this_thread::disable_interruption di;

		for(unsigned int i = 0; i < maxclients; i++)
		{
			if(!gSocket->IsClientConnected(i))
				continue;

			amxAsyncSession *sess = gPool->getClientSession(i);

			if((sess->pool().last_response + (ADDON_CLIENT_TIMEOUT_SECONDS * CLOCKS_PER_SEC)) < clock()) // 30 seconds
			{
				gSocket->KickClient(i, "boost::asio::deadline_timer: Timeout");
			}
		}

		boost::this_thread::restore_interruption re(di);
		boost::this_thread::sleep(boost::posix_time::seconds(5)); //boost::this_thread::sleep_for(boost::chrono::seconds(5));
	}
}



void amxAsyncServer::asyncAcceptor(unsigned int maxclients)
{
	amxAsyncSession *new_session = new amxAsyncSession(io_s, this);
	acceptor.async_accept(new_session->pool().sock, boost::bind(&amxAsyncServer::asyncHandler, this, new_session, boost::asio::placeholders::error, maxclients));
}



void amxAsyncServer::asyncHandler(amxAsyncSession *new_session, const boost::system::error_code& error, unsigned int maxclients)
{
	for(std::list<boost::asio::ip::address>::iterator i = connectedIPS.begin(); i != connectedIPS.end(); i++)
	{
		if(*i == new_session->pool().ip)
		{
			gDebug->Log("[RAW-NET] Cannot accept connection from %s: (What: IP already connected)", (*i).to_string().c_str());

			delete new_session;

			return asyncAcceptor(maxclients);
		}
	}

	cIPMutex.lock();
	connectedIPS.push_back(new_session->pool().ip);
	cIPMutex.unlock();

	if(!error)
	{
		unsigned int clientid = maxclients;

		for(unsigned int i = 0; i != maxclients; i++)
		{
			if(!gSocket->IsClientConnected(i))
			{
				clientid = i;

				break;
			}
		}

		if(clientid == maxclients)
		{
			gDebug->Log("[RAW-NET] Cannot accept connection from %s (What: Server is full)", new_session->pool().sock.remote_endpoint().address().to_string().c_str());

			amxCore::amxPush toAMX;

			toAMX.pushDataFirst.assign(new_session->pool().sock.remote_endpoint().address().to_string());
			toAMX.pushDataSecond.assign("boost::asio::async_acceptor: Server is full");

			gCore->pushToPT(ADDON_CALLBACK_OCCE, toAMX); // Addon_OnClientConnectError(remote_endpoint().address(), "Server is full");

			// server is full
			delete new_session;

			// sleep here if 'full DoS' damages cpu
			
			return asyncAcceptor(maxclients);
		}

		new_session->startSession(clientid);
	}
	else
	{
		gDebug->Log("Incoming connection from %s failed (What: %s)", new_session->pool().sock.remote_endpoint().address().to_string().c_str(), error.message().c_str());

		amxCore::amxPush toAMX;

		toAMX.pushDataFirst.assign(new_session->pool().sock.remote_endpoint().address().to_string());
		toAMX.pushDataSecond.assign(error.message());

		gCore->pushToPT(ADDON_CALLBACK_OCCE, toAMX); // Addon_OnClientConnectError(remote_endpoint().address(), error.message());

		delete new_session;
	}

	asyncAcceptor(maxclients);
}



void amxAsyncServer::sessionRemove(boost::asio::ip::address ip)
{
	cIPMutex.lock();
	connectedIPS.remove(ip);
	cIPMutex.unlock();
}



void amxAsyncSession::startSession(unsigned int binded_clid)
{
	poolHandle.ip = poolHandle.sock.remote_endpoint().address();
	poolHandle.remote_port = poolHandle.sock.remote_endpoint().port();
	poolHandle.connstate = 1;
	poolHandle.last_response = clock();

	gPool->setClientSession(binded_clid, this);
	gDebug->Log("Incoming connection from %s (binded clientid: %i)", poolHandle.ip.to_string().c_str(), binded_clid);

	amxCore::amxPush toAMX;

	toAMX.clientid = binded_clid;
	toAMX.pushDataFirst.assign(poolHandle.ip.to_string());

	gCore->pushToPT(ADDON_CALLBACK_OCC, toAMX); // Addon_OnClientConnect(binded_clid, poolHandle.ip);

	poolHandle.sock.async_read_some(boost::asio::buffer(poolHandle.buffer, sizeof poolHandle.buffer), boost::bind(&amxAsyncSession::readHandle, this, binded_clid, poolHandle.buffer, boost::asio::placeholders::error, boost::asio::placeholders::bytes_transferred));
}



void amxAsyncSession::readHandle(unsigned int clientid, const char *buffer, const boost::system::error_code& error, std::size_t bytes_rx)
{
	poolHandle.last_response = clock();

	if(!error)
	{
		std::istringstream input(buffer);
		std::string output;
		std::vector<std::string> args;

		int packet_crc;
		int remote_packet_crc;

		std::getline(input, output, '|');
			
		if(!amxString::isDecimial(output.c_str(), output.length()))
		{
			gDebug->Log("Client %i sent wrong/malformed packet (What: CRC is non-numeric)", clientid);
			gSocket->KickClient(clientid, "boost::asio::async_reader: Invalid packet CRC, NaN");

			return;
		}

		remote_packet_crc = atoi(output.c_str());

		std::getline(input, output);
		packet_crc = amxHash::crc32(output, output.length());

		if(remote_packet_crc != packet_crc)
		{
			gDebug->Log("Client %i sent wrong/malformed packet (What: CRC check failed [R:%i != L:%i])", clientid, remote_packet_crc, packet_crc);
			gSocket->KickClient(clientid, boost::str(boost::format("boost::asio::async_reader: Packet CRC check failed, [R:%1% != L:%2%]") % remote_packet_crc % packet_crc));

			return;
		}

		boost::split(args, output, boost::is_any_of("|"));

		if(poolHandle.connstate == 1) // RECEIVE TEMPLATE: "*HERE IS THE SERIAL*|*HERE IS ADDON VERSION*|*HERE IS SERVER IP CRC*|*HERE IS SERVER PORT*|*HERE IS CLIENT NAME*"
		{
			if(args.size() != 5)
			{
				gDebug->Log("Client %i sent wrong/malformed packet (What: Invalid argument count in connstate operation (%i))", clientid, args.size());
				gSocket->KickClient(clientid, boost::str(boost::format("boost::split::arg_parser: Invalid argument count while (connstate != 2) (%1%)") % args.size()));

				return;
			}

			unsigned short iter = 0;

			for(std::vector<std::string>::iterator i = args.begin(); i != args.end(); i++, iter++)
			{
				if(iter == 0) // SERIAL ID OF CLIENT
				{
					if((i->length() != 16) || !amxString::isHexDecimial(i->c_str(), i->length()))
					{
						gDebug->Log("Client %i sent wrong/malformed packet (What: Invalid serial ID)", clientid);
						gSocket->KickClient(clientid, "boost::split::arg_parser: Invalid serial ID");

						return;
					}

					poolHandle.sID.assign(*i);
					gDebug->Log("Client %i serial id is %s", clientid, poolHandle.sID.c_str());
				}
				else if(iter == 1) // ADDON VERSION ON CLIENT
				{
					if(!amxString::isDecimial(i->c_str(), i->length()))
					{
						gDebug->Log("Client %i sent wrong/malformed packet (What: Invalid addon version CRC, non-numeric)", clientid);
						gSocket->KickClient(clientid, "boost::split::arg_parser: Invalid CRC format");

						return;
					}

					poolHandle.addon_version_crc = atoi(i->c_str());
					gDebug->Log("Client %i addon version is %i", clientid, poolHandle.addon_version_crc);
				}
				else if(iter == 2) // OUR SERVER IP CRC
				{
					if(!amxString::isDecimial(i->c_str(), i->length()))
					{
						gDebug->Log("Client %i sent wrong/malformed packet (What: Invalid server IP CRC, non-numeric)", clientid);
						gSocket->KickClient(clientid, "boost::split::arg_parser: Invalid CRC format");

						return;
					}

					gDebug->Log("Client %i server ip CRC is %i", clientid, atoi(i->c_str()));
					//server ip check
				}
				else if(iter == 3) // OUR SERVER PORT
				{
					if(!amxString::isDecimial(i->c_str(), i->length()))
					{
						gDebug->Log("Client %i sent wrong/malformed packet (What: Invalid server port, non-numeric)", clientid);
						gSocket->KickClient(clientid, "boost::split::arg_parser: Invalid port format");

						return;
					}

					gDebug->Log("Client %i server port is %i", clientid, atoi(i->c_str()));
					// server port check
				}
				else if(iter == 4) // CLIENT PLAYER NAME
				{
					/*if(!(3 < i->length() < 20))
					{
						// invalid name length

						return;
					}*/

					if(!boost::regex_match(*i, boost::regex("[0-9a-zA-Z[\\]()$@._=]{3,20}")))
					{
						// invalid chars in name

						return;
					}

					poolHandle.name.assign(*i);
					gDebug->Log("Client %i player name is %s", clientid, poolHandle.name.c_str());
				}
				else
				{
					gDebug->Log("Internal plugin error, this case is impossible");

					return;
				}
			}

			poolHandle.connstate = 2; // must be 2

			// SEND TEMPLATE: "*HERE IS CLIENT REMOTE IP CRC*|*HERE IS CLIENT REMOTE PORT*"
			writeTo(clientid, boost::str(boost::format("%1%|%2%") % amxHash::crc32(poolHandle.ip.to_string(), poolHandle.ip.to_string().length()) % poolHandle.remote_port));

			return;
		}

		poolHandle.pqMutex.lock();
		poolHandle.pendingQueue.push(args);
		poolHandle.pqMutex.unlock();

		gDebug->Log("Got '%s' (length: %i) from client %i", buffer, bytes_rx, clientid);
	}
	else
	{
		gDebug->Log("Cannot read data from client %i (What: %i %s)", clientid, error.value(), error.message().c_str());

		amxCore::amxPush toAMX;

		toAMX.clientid = clientid;
		toAMX.pushDataFirst.assign(poolHandle.ip.to_string());
		toAMX.pushDataSecond.assign(boost::str(boost::format("%1%|%2%") % error.value() % error.message()));

		gCore->pushToPT(ADDON_CALLBACK_OCD, toAMX);
		gPool->resetOwnSession(clientid);
	}
}



void amxAsyncSession::writeTo(unsigned int clientid, std::string data)
{
	if(!gSocket->IsClientConnected(clientid))
		return;

	int packet_crc = amxHash::crc32(data, data.length());
	char p_crc[15];

	#if defined LINUX
	snprintf(p_crc, sizeof p_crc, "%i", packet_crc);
	#else
	itoa(packet_crc, p_crc, 10);
	#endif

	strcat(p_crc, "|");
	data.insert(0, p_crc);

	gDebug->Log("Sending '%s' (length: %i) to client %i", data.c_str(), data.length(), clientid);

	amxAsyncSession *sessid = gPool->getClientSession(clientid);
	sessid->pool().sock.async_write_some(boost::asio::buffer(data, data.length()), boost::bind(&amxAsyncSession::writeHandle, sessid, clientid, boost::asio::placeholders::error, boost::asio::placeholders::bytes_transferred));
}



void amxAsyncSession::writeHandle(unsigned int clientid, const boost::system::error_code& error, std::size_t bytes_tx)
{
	if(!error)
	{
		gDebug->Log("Sent %i bytes to client %i", bytes_tx, clientid);

		memset(poolHandle.buffer, NULL, sizeof poolHandle.buffer);
		poolHandle.sock.async_read_some(boost::asio::buffer(poolHandle.buffer, sizeof poolHandle.buffer), boost::bind(&amxAsyncSession::readHandle, this, clientid, poolHandle.buffer, boost::asio::placeholders::error, boost::asio::placeholders::bytes_transferred));
	}
	else
	{
		gDebug->Log("Cannot send data to client %i (What: %i %s)", clientid, error.value(), error.message().c_str());

		amxCore::amxPush toAMX;

		toAMX.clientid = clientid;
		toAMX.pushDataFirst.assign(poolHandle.ip.to_string());
		toAMX.pushDataSecond.assign(boost::str(boost::format("%1%|%2%") % error.value() % error.message()));

		gCore->pushToPT(ADDON_CALLBACK_OCD, toAMX);
		gPool->resetOwnSession(clientid);
	}
}