#pragma once



#include "server.hpp"





boost::shared_ptr<amxTransfer> gTransfer;


extern boost::shared_ptr<amxCore> gCore;
extern boost::shared_ptr<amxDebug> gDebug;
extern boost::shared_ptr<amxPool> gPool;
extern boost::shared_ptr<amxSocket> gSocket;





amxTransfer::amxTransfer(bool local, unsigned int clientid, std::string filename, std::string remote_filename, std::size_t remote_file_size, int remote_file_crc) : file_name(filename), remote_file_name(remote_filename), is_local(local)
{
	gDebug->Log("Transfer constructor called");

	file_size = NULL;
	file_crc = NULL;

	if(is_local)
	{
		file.open(file_name.c_str(), (std::fstream::in | std::fstream::binary | std::fstream::ate));
		boost::thread s(boost::bind(&amxTransfer::processSend, this, clientid, boost::system::error_code::error_code(), NULL));
	}
	else
	{
		file_size = remote_file_size;
		file_crc = remote_file_crc;

		file.open(file_name.c_str(), (std::fstream::out | std::fstream::binary));
		boost::thread r(boost::bind(&amxTransfer::processRecv, this, clientid, filename.c_str(), boost::system::error_code::error_code(), NULL));
	}
}



amxTransfer::~amxTransfer()
{
	gDebug->Log("Transfer destructor called");
}



void amxTransfer::processSend(unsigned int clientid, const boost::system::error_code& error, std::size_t bytes_tx)
{
	if(!bytes_tx && !file_size)
	{
		boost::this_thread::sleep(boost::posix_time::seconds(1));

		file_size = file.tellg();
		file_buf = new char[file_size + 1];
		memset(file_buf, NULL, file_size);

		file.seekg(std::fstream::beg);
		file.read(file_buf, file_size);
		file.close();

		boost::asio::async_write(gPool->getClientSession(clientid)->pool().sock, boost::asio::buffer(file_buf, file_size), boost::asio::transfer_exactly(file_size), boost::bind(&amxTransfer::processSend, this, clientid, boost::asio::placeholders::error, boost::asio::placeholders::bytes_transferred));

		return;
	}

	amxCore::amxPush toAMX;
	amxPool::svrData toData;

	if(!error)
	{
		gDebug->Log("ProcessSend: Wrote %i bytes with no error (file size is %i)", bytes_tx, file_size);

		toAMX.clientid = clientid;

		toData.reset();
		toData.string.assign(file_name);
		toAMX.args.push_back(toData);

		toData.reset();
		toData.string.assign(remote_file_name);
		toAMX.args.push_back(toData);

		toData.reset();
		toData.integer = bytes_tx;
		toAMX.args.push_back(toData);

		gCore->pushToPT(ADDON_CALLBACK_OLFT, toAMX);
	}
	else
	{
		gDebug->Log("ProcessSend: Error while transfering (%i bytes processed, file size is %i): %s", bytes_tx, file_size, error.message().c_str());
		
		toAMX.clientid = clientid;

		toData.reset();
		toData.string.assign(file_name);
		toAMX.args.push_back(toData);

		toData.reset();
		toData.string.assign(remote_file_name);
		toAMX.args.push_back(toData);

		toData.reset();
		toData.integer = error.value();
		toAMX.args.push_back(toData);

		toData.reset();
		toData.string.assign(error.message());
		toAMX.args.push_back(toData);

		gCore->pushToPT(ADDON_CALLBACK_OLFTE, toAMX);
	}

	delete[] file_buf;
	gPool->getClientSession(clientid)->pool().file_t = false;
}



void amxTransfer::processRecv(unsigned int clientid, const char *buffer, const boost::system::error_code& error, std::size_t bytes_rx)
{
	if(!bytes_rx)
	{
		boost::this_thread::sleep(boost::posix_time::seconds(1));

		file_buf = new char[file_size + 1];
		memset(file_buf, NULL, file_size);

		boost::asio::async_read(gPool->getClientSession(clientid)->pool().sock, boost::asio::buffer(file_buf, file_size), boost::asio::transfer_exactly(file_size), boost::bind(&amxTransfer::processRecv, this, clientid, file_buf, boost::asio::placeholders::error, boost::asio::placeholders::bytes_transferred));

		return;
	}

	amxCore::amxPush toAMX;
	amxPool::svrData toData;

	if(!error)
	{
		gDebug->Log("Processed %i bytes of %i", bytes_rx, file_size);

		if(bytes_rx == file_size)
		{
			file.write(buffer, bytes_rx);
			file.close();

			if(file_crc == amxHash::crc32_file(file_name))
			{
				// success
				gDebug->Log("ProcessRecv: Read %i bytes with no error (file size is %i)", bytes_rx, file_size);

				toAMX.clientid = clientid;

				toData.reset();
				toData.string.assign(file_name);
				toAMX.args.push_back(toData);

				toData.reset();
				toData.string.assign(remote_file_name);
				toAMX.args.push_back(toData);

				toData.reset();
				toData.integer = bytes_rx;
				toAMX.args.push_back(toData);

				gCore->pushToPT(ADDON_CALLBACK_ORFT, toAMX);
			}
			else
			{
				gDebug->Log("ProcessRecv: Error while transfering %s (final CRC check failed [L:%i != R:%i])", file_name.c_str(), amxHash::crc32_file(file_name), file_crc);

				toAMX.clientid = clientid;

				toData.reset();
				toData.string.assign(remote_file_name);
				toAMX.args.push_back(toData);

				toData.reset();
				toData.string.assign(file_name);
				toAMX.args.push_back(toData);

				toData.reset();
				toData.integer = 1488;
				toAMX.args.push_back(toData);

				toData.reset();
				toData.string.assign("CRC check failed");
				toAMX.args.push_back(toData);

				gCore->pushToPT(ADDON_CALLBACK_ORFTE, toAMX);
			}
		}
		else
		{
			gDebug->Log("Error");
		}
	}
	else
	{
		gDebug->Log("ProcessRecv: Error while transfering %s (%i bytes processed, file size is %i): %s", file_name.c_str(), bytes_rx, file_size, error.message().c_str());
		
		toAMX.clientid = clientid;

		toData.reset();
		toData.string.assign(remote_file_name);
		toAMX.args.push_back(toData);

		toData.reset();
		toData.string.assign(file_name);
		toAMX.args.push_back(toData);

		toData.reset();
		toData.integer = error.value();
		toAMX.args.push_back(toData);

		toData.reset();
		toData.string.assign(error.message());
		toAMX.args.push_back(toData);

		gCore->pushToPT(ADDON_CALLBACK_ORFTE, toAMX);
	}

	delete[] file_buf;
	gPool->getClientSession(clientid)->pool().file_t = false;
}