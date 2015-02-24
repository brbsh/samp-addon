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

	threadInstance = boost::shared_ptr<boost::thread>(new boost::thread(boost::bind(&amxSocket::acceptThread, this, ip, port, maxclients, worker_count)));
	/*threadGroup = boost::shared_ptr<boost::thread_group>(new boost::thread_group());

	for(short i = 0; i < worker_count; i++)
		threadGroup->create_thread(boost::bind(&amxSocket::acceptThread, ip, port, maxclients, (i + 1)));*/
}



amxSocket::~amxSocket()
{
	gDebug->Log("Socket destructor called");

	threadInstance->interrupt();
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
	amxPool::svrData toData;
	amxAsyncSession *sess = gPool->getClientSession(clientid);

	toAMX.clientid = clientid;

	toData.reset();
	toData.string.assign(sess->pool().ip.to_string());
	toAMX.args.push_back(toData);

	toData.reset();
	toData.integer = 1488;
	toAMX.args.push_back(toData);

	toData.reset();
	toData.string.assign(reason);
	toAMX.args.push_back(toData);

	gCore->pushToPT(ADDON_CALLBACK_OCD, toAMX);
	gDebug->Log("Client %i was kicked (Reason: %s)", clientid, reason.c_str());

	sess->pool().sock.shutdown(boost::asio::socket_base::shutdown_both);
	gPool->resetOwnSession(clientid);
}



void amxSocket::acceptThread(amxSocket *instance, std::string ip, unsigned short port, unsigned int maxclients, unsigned short workerID)
{
	gDebug->Log("Thread amxSocket::acceptThread(worker #%i) successfuly started", workerID);

	boost::asio::io_service io_service;

	while(gPool->getPluginStatus())
	{
		instance->serverHandle = boost::shared_ptr<amxAsyncServer>(new amxAsyncServer(io_service, ip, port, maxclients));
		amxCore::amxPush toAMX;
		amxPool::svrData toData;

		try
		{
			toAMX.clientid = workerID;

			gCore->pushToPT(ADDON_CALLBACK_OTWS, toAMX); // Addon_OnTCPWorkerStarted(workerID);
			gDebug->Log("TCP worker #%i started on %s:%i with max connections: %i", workerID, ip.c_str(), port, maxclients);
			io_service.run(); // blocking thread, releasing when error occurs
		}
		catch(boost::system::system_error& error)
		{
			gDebug->Log("Error while processing TCP worker #%i execution (What: %s)", workerID, error.what());

			toAMX.clientid = workerID;

			toData.reset();
			toData.integer = error.code().value();
			toAMX.args.push_back(toData);
			
			toData.reset();
			toData.string.assign(error.what());
			toAMX.args.push_back(toData);

			gCore->pushToPT(ADDON_CALLBACK_OTWE, toAMX); // Addon_OnTCPWorkerError(workerID, error_code, err.what());
		}

		io_service.stop();
		io_service.reset();
		instance->serverHandle.reset();

		gDebug->Log("Restarting TCP worker #%i due to stop detected...", workerID);

		boost::this_thread::sleep(boost::posix_time::seconds(5)); // boost::this_thread::sleep_for(boost::chrono::seconds(5));
	}
}



void amxAsyncServer::asyncAcceptor(unsigned int maxclients)
{
	amxAsyncSession *new_session = new amxAsyncSession(io_s, this);
	acceptor.async_accept(new_session->pool().sock, boost::bind(&amxAsyncServer::asyncHandler, this, new_session, boost::asio::placeholders::error, maxclients));
}



void amxAsyncServer::asyncHandler(amxAsyncSession *new_session, const boost::system::error_code& error, unsigned int maxclients)
{
	if(!error)
	{
		for(boost::unordered_map<unsigned int, boost::asio::ip::address>::iterator i = sessions.begin(); i != sessions.end(); i++)
		{
			if((i->second == new_session->pool().sock.remote_endpoint().address()) && !gSocket->IsClientConnected(i->first))
			{
				new_session->startSession(i->first);

				return asyncAcceptor(maxclients);
			}
		}

		delete new_session;
		return asyncAcceptor(maxclients);
	}
	else
	{
		gDebug->Log("Incoming connection from %s failed (What: %s)", new_session->pool().sock.remote_endpoint().address().to_string().c_str(), error.message().c_str());

		amxCore::amxPush toAMX;
		amxPool::svrData toData;

		toData.reset();
		toData.string.assign(new_session->pool().sock.remote_endpoint().address().to_string());
		toAMX.args.push_back(toData);
		
		toData.reset();
		toData.integer = error.value();
		toAMX.args.push_back(toData);

		toData.reset();
		toData.string.assign(error.message());
		toAMX.args.push_back(toData);

		gCore->pushToPT(ADDON_CALLBACK_OCCE, toAMX); // Addon_OnClientConnectError(remote_endpoint().address(), error.value(), error.message());

		delete new_session;
	}

	asyncAcceptor(maxclients);
}



void amxAsyncServer::sessionAdd(unsigned int clientid, boost::asio::ip::address ip)
{
	boost::unique_lock<boost::mutex> lockit(sMutex);
	sessions[clientid] = ip;
}



void amxAsyncServer::sessionRemove(unsigned int clientid)
{
	boost::unique_lock<boost::mutex> lockit(sMutex);
	sessions.erase(clientid);
}



void amxAsyncSession::startSession(unsigned int binded_clid)
{
	poolHandle.ip = poolHandle.sock.remote_endpoint().address();
	poolHandle.remote_port = poolHandle.sock.remote_endpoint().port();
	poolHandle.connstate = 1;
	poolHandle.file_t = false;

	gPool->setClientSession(binded_clid, this);
	gDebug->Log("Incoming connection from %s (binded clientid: %i)", poolHandle.ip.to_string().c_str(), binded_clid);

	amxCore::amxPush toAMX;
	amxPool::svrData toData;

	toAMX.clientid = binded_clid;

	toData.reset();
	toData.string.assign(poolHandle.ip.to_string());
	toAMX.args.push_back(toData);

	gCore->pushToPT(ADDON_CALLBACK_OCC, toAMX); // Addon_OnClientConnect(binded_clid, poolHandle.ip);

	poolHandle.sock.async_write_some(boost::asio::buffer("ACCEPTED"), boost::bind(&amxAsyncSession::writeHandle, this, binded_clid, boost::asio::placeholders::error, boost::asio::placeholders::bytes_transferred));
	//poolHandle.sock.async_read_some(boost::asio::buffer(poolHandle.buffer, sizeof poolHandle.buffer), boost::bind(&amxAsyncSession::readHandle, this, binded_clid, poolHandle.buffer, boost::asio::placeholders::error, boost::asio::placeholders::bytes_transferred));
}



void amxAsyncSession::readHandle(unsigned int clientid, const char *buffer, const boost::system::error_code& error, std::size_t bytes_rx)
{
	if(!error)
	{
		if(poolHandle.file_t)
		{
			if(poolHandle.fileT->isLocal())
			{
				gDebug->Log("Client %i sent data while file transfer processing: %s (length: %i)", buffer, bytes_rx);

				return;
			}
			else
			{

			}
		}

		gDebug->Log("Got '%s' (length: %i) from client %i", buffer, bytes_rx, clientid);

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

		std::getline(input, output, '\0');
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
					
					amxPool::svrData sData;
					sData = gPool->getServerVar("ip");

					int server_crc = amxHash::crc32(sData.string, sData.string.length());
					int remote_server_crc = atoi(i->c_str());

					gDebug->Log("Server ip CRC is %i", server_crc);
					gDebug->Log("Client %i server ip CRC is %i", clientid, remote_server_crc);

					/*if(server_crc != remote_server_crc)
					{
						// CRC missmatch
						gDebug->Log("Client %i sent wrong/malformed packet (What: server IP CRC match failed [L:%i != R:%i]", clientid, server_crc, remote_server_crc);
						gSocket->KickClient(clientid, boost::str(boost::format("boost::split::arg_parser: server CRC match failed [L:%1% != R:%2%]") % server_crc % remote_server_crc));
						
						return;
					}*/

				}
				else if(iter == 3) // OUR SERVER PORT
				{
					if(!amxString::isDecimial(i->c_str(), i->length()))
					{
						gDebug->Log("Client %i sent wrong/malformed packet (What: Invalid server port, non-numeric)", clientid);
						gSocket->KickClient(clientid, "boost::split::arg_parser: Invalid port format");

						return;
					}

					amxPool::svrData sData;
					sData = gPool->getServerVar("port");

					int remote_server_port = atoi(i->c_str());

					gDebug->Log("Server port is %i", sData.integer);
					gDebug->Log("Client %i server port is %i", clientid, remote_server_port);

					if(sData.integer != remote_server_port)
					{
						// server port invalid
						gSocket->KickClient(clientid, boost::str(boost::format("boost::split::arg_parser: server port match failed [L:%1% != R:%2%]") % sData.integer % remote_server_port));

						return;
					}
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
			char buffer_[2048];

			memset(buffer_, NULL, sizeof(buffer_));
			poolHandle.sock.async_read_some(boost::asio::buffer(buffer_), boost::bind(&amxAsyncSession::readHandle, this, clientid, buffer_, boost::asio::placeholders::error, boost::asio::placeholders::bytes_transferred));

			return;
		}

		poolHandle.pqMutex.lock();
		poolHandle.pendingQueue.push(args);
		poolHandle.pqMutex.unlock();
	}
	else
	{
		gDebug->Log("Cannot read data from client %i (What: %i %s)", clientid, error.value(), error.message().c_str());

		amxCore::amxPush toAMX;
		amxPool::svrData toData;

		toAMX.clientid = clientid;

		toData.reset();
		toData.string.assign(poolHandle.ip.to_string());
		toAMX.args.push_back(toData);

		toData.reset();
		toData.integer = error.value();
		toAMX.args.push_back(toData);

		toData.reset();
		toData.string.assign(error.message());
		toAMX.args.push_back(toData);

		gCore->pushToPT(ADDON_CALLBACK_OCD, toAMX);
		gPool->resetOwnSession(clientid);
	}
}



void amxAsyncSession::writeTo(unsigned int clientid, std::string data)
{
	if(!gSocket->IsClientConnected(clientid))
		return;

	amxAsyncSession *sessid = gPool->getClientSession(clientid);

	if(sessid->pool().file_t)
		return;

	data.insert(0, boost::str(boost::format("%1%|") % amxHash::crc32(data, data.length())));
	gDebug->Log("Sending '%s' (length: %i) to client %i", data.c_str(), data.length(), clientid);

	
	sessid->pool().sock.async_write_some(boost::asio::buffer(data, data.length()), boost::bind(&amxAsyncSession::writeHandle, sessid, clientid, boost::asio::placeholders::error, boost::asio::placeholders::bytes_transferred));
}



void amxAsyncSession::writeHandle(unsigned int clientid, const boost::system::error_code& error, std::size_t bytes_tx)
{
	if(!error)
	{
		char buffer[2048];

		gDebug->Log("Sent %i bytes to client %i", bytes_tx, clientid);
		memset(buffer, NULL, sizeof(buffer));
		poolHandle.sock.async_read_some(boost::asio::buffer(buffer, sizeof(buffer)), boost::bind(&amxAsyncSession::readHandle, this, clientid, buffer, boost::asio::placeholders::error, boost::asio::placeholders::bytes_transferred));
	}
	else
	{
		gDebug->Log("Cannot send data to client %i (What: %i %s)", clientid, error.value(), error.message().c_str());

		amxCore::amxPush toAMX;
		amxPool::svrData toData;

		toAMX.clientid = clientid;

		toData.reset();
		toData.string.assign(poolHandle.ip.to_string());
		toAMX.args.push_back(toData);

		toData.reset();
		toData.integer = error.value();
		toAMX.args.push_back(toData);

		toData.reset();
		toData.string.assign(error.message());
		toAMX.args.push_back(toData);

		gCore->pushToPT(ADDON_CALLBACK_OCD, toAMX);
		gPool->resetOwnSession(clientid);
	}
}