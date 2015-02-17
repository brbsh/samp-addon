#pragma once



#include "server.hpp"





boost::shared_ptr<amxSocket> gSocket;


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



void amxSocket::KickClient(unsigned int clientid)
{
	if(!IsClientConnected(clientid))
		return;

	gPool->resetOwnSession(clientid);
	gDebug->Log("Client %i was kicked", clientid);
}



void amxSocket::acceptThread(std::string ip, unsigned short port, unsigned int maxclients, unsigned short workerID)
{
	gDebug->Log("Thread amxSocket::acceptThread(worker #%i) successfuly started", workerID);

	boost::asio::io_service io_service;

	while(gPool->getPluginStatus())
	{
		try
		{
			amxAsyncServer server(io_service, ip, port, maxclients);
			gDebug->Log("TCP worker #%i started on %s:%i with max connections: %i", workerID, ip.c_str(), port, maxclients);
			io_service.run();
		}
		catch(boost::system::system_error& err)
		{
			gDebug->Log("Error while processing TCP worker #%i execution (What: %s)", workerID, err.what());
		}

		gDebug->Log("Restarting TCP worker #%i due to stop detected...", workerID);
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
				gDebug->Log("Client %i was kicked (What: 30 seconds timeout)", i);
				// 30 sec timeout
				gSocket->KickClient(i);
			}
		}

		boost::this_thread::restore_interruption re(di);
		boost::this_thread::sleep_for(boost::chrono::seconds(5));
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
		if(*i == new_session->pool().sock.remote_endpoint().address())
		{
			gDebug->Log("Cannot accept connection from %s: (What: IP already connected)", (*i).to_string().c_str());

			delete new_session;

			return asyncAcceptor(maxclients);
		}
	}

	cIPMutex.lock();
	connectedIPS.push_back(new_session->pool().sock.remote_endpoint().address());
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
			gDebug->Log("Cannot accept connection from %s (What: Server is full)", new_session->pool().sock.remote_endpoint().address().to_string().c_str());

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
	poolHandle.ip = poolHandle.sock.remote_endpoint().address().to_string();
	poolHandle.remote_port = poolHandle.sock.remote_endpoint().port();
	poolHandle.connstate = 1;
	poolHandle.last_response = clock();

	gPool->setClientSession(binded_clid, this);
	gDebug->Log("Incoming connection from %s (binded clientid: %i)", poolHandle.ip.c_str(), binded_clid);

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
			gSocket->KickClient(clientid);

			return;
		}

		remote_packet_crc = atoi(output.c_str());

		std::getline(input, output);
		packet_crc = amxHash::crc32(output, output.length());

		if(remote_packet_crc != packet_crc)
		{
			gDebug->Log("Client %i sent wrong/malformed packet (What: CRC check failed [R:%i != L:%i])", clientid, remote_packet_crc, packet_crc);
			gSocket->KickClient(clientid);

			return;
		}

		gDebug->Log("CRC check passed (R:%i = L:%i)", remote_packet_crc, packet_crc);
		boost::split(args, output, boost::is_any_of("|"));

		if(poolHandle.connstate == 1) // RECEIVE TEMPLATE: "*HERE IS THE SERIAL*|*HERE IS ADDON VERSION*|*HERE IS SERVER IP CRC*|*HERE IS SERVER PORT*|*HERE IS CLIENT NAME*"
		{
			if(args.size() != 5)
			{
				gDebug->Log("Client %i sent wrong/malformed packet (What: Invalid argument count in connstate operation (%i))", clientid, args.size());
				gSocket->KickClient(clientid);

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
						gSocket->KickClient(clientid);

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
						gSocket->KickClient(clientid);

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
						gSocket->KickClient(clientid);

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
						gSocket->KickClient(clientid);

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
			writeTo(clientid, boost::str(boost::format("%1%|%2%") % amxHash::crc32(poolHandle.ip, poolHandle.ip.length()) % poolHandle.remote_port));

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
		memset(poolHandle.buffer, NULL, sizeof poolHandle.buffer);
		poolHandle.sock.async_read_some(boost::asio::buffer(poolHandle.buffer, sizeof poolHandle.buffer), boost::bind(&amxAsyncSession::readHandle, this, clientid, poolHandle.buffer, boost::asio::placeholders::error, boost::asio::placeholders::bytes_transferred));
	}
	else
	{
		gDebug->Log("Cannot send data to client %i (What: %i %s)", clientid, error.value(), error.message().c_str());
		gPool->resetOwnSession(clientid);
	}
}