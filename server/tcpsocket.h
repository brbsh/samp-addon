#pragma once



#include "server.h"





class amxSocket
{

public:

	amxSocket(std::string ip, int port, int maxclients);
	~amxSocket();

	bool IsClientConnected(int clientid);
	void KickClient(int clientid);
	void Send(int clientid, std::string data);

	static void Thread();
	static void SendThread();
	static void ReceiveThread(int clientid);

	bool Active;
	int Port;
	int MaxClients;

	std::string IP;
	boost::mutex Mutex;

};