#pragma once



#include "httpclient.h"





addonHTTP *gHTTP;

extern addonSocket *gSocket;





std::string addonHTTP::HTTP(std::string host, std::string query)
{
	boost::mutex hMutex;
	boost::system::error_code error;
	std::stringstream ret;

	boost::mutex::scoped_lock lock(hMutex);
	boost::asio::ip::tcp::resolver resolver(gSocket->io);
	lock.unlock();

	boost::asio::ip::tcp::resolver::query q(host, "http");

	boost::asio::ip::tcp::resolver::iterator endpoint = resolver.resolve(q, error);
	boost::asio::ip::tcp::resolver::iterator end;

	boost::asio::ip::tcp::socket socket(gSocket->io);
	error = boost::asio::error::host_not_found;

	while(error && (endpoint != end))
	{
		socket.close();
		socket.connect(*endpoint++, error);
	}

	if(error)
	{
		addonDebug("Cannot connect to %s:80", host.c_str());

		return "";
	}

	boost::asio::streambuf request;
	std::ostream request_stream(&request);

	request_stream << "GET " << query << " HTTP/1.0\r\n";
	request_stream << "Host: " << host << "\r\n";
	request_stream << "Accept: */*\r\n";
	request_stream << "Connection: close\r\n\r\n";

	boost::asio::write(socket, request);

	boost::asio::streambuf response;
	boost::asio::read_until(socket, response, "\r\n");

	unsigned int response_code = NULL;

	std::istream response_stream(&response);
	std::string http_version;
	std::string response_message;

	response_stream >> http_version;
	response_stream >> response_code;
	std::getline(response_stream, response_message);

	if(!response_stream || (http_version.substr(NULL, 5) != "HTTP/") || response_code != 200)
	{
		addonDebug("Invalid response from %s", host.c_str());

		return "";
	}

	boost::asio::read_until(socket, response, "\r\n\r\n");

	std::string header;

	while(std::getline(response_stream, header) && (header != "\r"))
	{

	}

	if(response.size() > 0)
		ret << &response;

	while(boost::asio::read(socket, response, boost::asio::transfer_at_least(1), error))
	{
		if(error)
		{
			addonDebug("Transfer error");

			return "";
		}

		ret << &response;
	}

	socket.close();

	return ret.str();
}