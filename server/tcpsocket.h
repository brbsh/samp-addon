#pragma once



#include "addon.h"





class amxSocket
{

public:

	static void Thread();
	static void SendThread();
	static void ReceiveThread();

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

	boost::unordered_map<int, boost::shared_ptr<boost::asio::ip::tcp::socket> > Socket;
};



struct processStruct
{
	int clientID;

	std::string data;
};