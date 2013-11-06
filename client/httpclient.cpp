#pragma once



#include "httpclient.h"





extern boost::asio::io_service gIOService;
extern boost::shared_ptr<addonDebug> gDebug;
extern boost::shared_ptr<addonSocket> gSocket;





std::string addonHTTP::Get(std::string host, std::string query)
{
	gDebug->traceLastFunction("addonHTTP::Get(host = '%s', query = '%s') at 0x%x", host.c_str(), query.c_str(), &addonHTTP::Get);

    boost::system::error_code error;
    std::stringstream ret;

    boost::asio::ip::tcp::resolver resolver(gIOService);
    boost::asio::ip::tcp::resolver::query q(host, "http");

    boost::asio::ip::tcp::resolver::iterator endpoint = resolver.resolve(q, error);
    boost::asio::ip::tcp::resolver::iterator end;

    boost::asio::ip::tcp::socket socket(gIOService);
    error = boost::asio::error::host_not_found;

    while(error && (endpoint != end))
    {
        socket.close();
        socket.connect(*endpoint++, error);
	}

    if(error)
    {
        gDebug->Log("Cannot connect to %s:80", host.c_str());

        return "\0";
    }

    boost::asio::streambuf request;
    std::ostream request_stream(&request);

    request_stream << "GET " << query << " HTTP/1.1\r\n";
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
        gDebug->Log("Invalid response from %s:80", host.c_str());

        return "\0";
    }

    boost::asio::read_until(socket, response, "\r\n\r\n");

    std::string header;

    while(std::getline(response_stream, header) && (header != "\r"))
    {
		boost::this_thread::sleep_for(boost::chrono::milliseconds(1));
    }

    if(response.size() > 0)
		ret << &response;

    while(boost::asio::read(socket, response, boost::asio::transfer_at_least(1), error))
    {
        if(error)
        {
            gDebug->Log("Transfer error");

            return "\0";
        }

        ret << &response;
    }

    socket.close();

	return ret.str();
}