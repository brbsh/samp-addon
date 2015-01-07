#pragma once



#include "server.h"





class amxSocket
{

public:

	boost::shared_ptr<boost::thread> acceptThread;
	boost::shared_ptr<boost::thread> sendThread;

	amxSocket(std::string ip, int port, int maxclients);
	virtual ~amxSocket();

	//bool IsClientConnected(int clientid);
	//void KickClient(int clientid);
	//void Send(int clientid, std::string data);

	static void AcceptThread(std::string ip, int port, int maxClients);
	static void SendThread();
	//static void ReceiveThread(int clientid);
};