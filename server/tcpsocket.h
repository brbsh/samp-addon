#pragma once



#include "addon.h"





struct socketStruct
{
	int maxClients;
	bool active;
};



class amxSocket
{

public:

	boost::mutex Mutex;

	static void ConnectionThread(int socketid);
	static void SendThread(int socketid);
	static void ReceiveThread(int socketid);

	struct socketStruct socketInfo;

	amxSocket();
	~amxSocket();

	void Close();
	void CloseSocket(int socketid);
	void MaxClients(int max);
	int FindFreeSlot();
	void Bind(std::string ip);
	void Listen(int port);
	void KickClient(int clientid);
	bool IsClientConnected(int clientid);
	std::string GetClientIP(int clientid);
	void Send(int clientid, std::string data);
	
	int SetNonblock(int sockid);

private:

	int socketID;
	std::string bind_ip;

	#ifdef WIN32
		HANDLE sendHandle;
		HANDLE recvHandle;
		HANDLE connHandle;
	#else
		pthread_t sendHandle;
		pthread_t recvHandle;
		pthread_t connHandle;
	#endif
		
	void Create();
};



struct processStruct
{
	int clientID;
	std::string data;
};