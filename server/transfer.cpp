#pragma once



#include "server.hpp"





boost::shared_ptr<amxTransfer> gTransfer;


extern boost::shared_ptr<amxPool> gPool;
extern boost::shared_ptr<amxSocket> gSocket;





amxTransfer::amxTransfer(std::string ip, unsigned short port)
{

}



amxTransfer::~amxTransfer()
{

}