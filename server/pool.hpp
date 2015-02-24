#pragma once



#include "server.hpp"





class amxPool
{

public:

	struct clientPoolS
	{
		clientPoolS(boost::asio::io_service& io_service) : sock(io_service)
		{
			connstate = NULL;
			sID.clear();
		}

		~clientPoolS()
		{

		}

		boost::asio::ip::tcp::socket sock;
		boost::asio::ip::address ip;

		std::queue< std::vector<std::string> > pendingQueue;
		boost::mutex pqMutex;

		unsigned short connstate;
		unsigned short remote_port;
		int addon_version_crc;
		int cmdresponse_state;

		std::string sID;
		std::string name;

		bool file_t;
		amxTransfer *fileT;
	};

	struct svrData
	{
		long integer;
		double floating;
		std::string string;

		void reset()
		{
			integer = 0;
			floating = 0.0f;
			string.clear();
		}
	};

	amxPool();
	virtual ~amxPool();

	void setPluginStatus(bool status);
	bool getPluginStatus();

	void setServerVar(std::string key, svrData struc);
	svrData getServerVar(std::string key);

	unsigned int activeSessions();
	void resetOwnSession(unsigned int clientid);
	bool hasOwnSession(unsigned int clientid);
	void setClientSession(unsigned int clientid, amxAsyncSession *session);
	amxAsyncSession *getClientSession(unsigned int clientid);

private:

	bool pluginState;
	boost::shared_mutex pstMutex;

	boost::unordered_map<std::string, svrData> serverPool;
	boost::shared_mutex spMutex;

	boost::unordered_map<unsigned int, amxAsyncSession *> clientPool;
	boost::shared_mutex cpMutex;
};