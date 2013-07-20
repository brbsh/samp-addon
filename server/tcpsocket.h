#pragma once



#include "addon.h"





class amxSocket
{

public:

	static void Thread();
	static void SendThread();
	static void ReceiveThread(int clientid);

	amxSocket(std::string ip, int port, int maxclients);
	~amxSocket();

	bool IsClientConnected(int clientid);
	void KickClient(int clientid);
	void Send(int clientid, std::string data);

	bool Active;
	int Port;
	int MaxClients;

	std::string IP;
	boost::mutex Mutex;

	boost::asio::io_service io;
};



struct processStruct
{
	int clientID;

	std::string data;
};