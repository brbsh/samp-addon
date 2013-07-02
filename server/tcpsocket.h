#pragma once



#include "addon.h"





class amxSocket
{

public:

	boost::mutex Mutex;
	boost::unordered_map<int, boost::shared_ptr<boost::asio::ip::tcp::socket> > Socket;

	bool Active;
	int Port;
	int MaxConnections;

	static void Thread();
	static void ClientThread(int clientid);

	amxSocket(int port, int maxclients);
	~amxSocket();
};



struct processStruct
{
	int clientID;
	std::string data;
};