#pragma once



#include "client.hpp"





class addonAsyncHTTP
{

public:

	addonAsyncHTTP(boost::asio::io_service& io_service, std::string host, std::string query) : io_s(io_service), resolr(io_service), sock(io_service), readyFlag(false)
	{
		response.clear();
		response_code = -1;

		std::ostream rqStream(&req);

		rqStream << "GET " << query << " HTTP/1.1\r\n";
		rqStream << "User-Agent: boost/1.49.0 (WIN32; WIN64) SAMP-Addon/2015.02.19\r\n";
		rqStream << "Host: " << host << "\r\n";
		rqStream << "Accept: */*\r\n";
		rqStream << "Connection: close\r\n\r\n";

		boost::asio::ip::tcp::resolver::query res_query(host, "http");
		resolr.async_resolve(res_query, boost::bind(&addonAsyncHTTP::resolveHandle, this, boost::asio::placeholders::error, boost::asio::placeholders::iterator));
		asyncOPThread = boost::shared_ptr<boost::thread>(new boost::thread(boost::bind(&addonAsyncHTTP::httpThread, this)));
	}

	~addonAsyncHTTP()
	{
		delete[] result;

		io_s.stop();
		io_s.reset();
		sock.close();

		asyncOPThread->interrupt();
	}

	void httpThread()
	{
		try
		{
			io_s.run();
		}
		catch(boost::system::system_error& error)
		{
			response.assign(error.what());
			response_code = error.code().value();
			// catch error
		}
	}

	void resolveHandle(const boost::system::error_code& error, boost::asio::ip::tcp::resolver::iterator endpoint_iter)
	{
		boost::asio::ip::tcp::endpoint endp = *endpoint_iter;
		sock.async_connect(endp, boost::bind(&addonAsyncHTTP::connectHandle, this, boost::asio::placeholders::error, ++endpoint_iter));
	}

	void connectHandle(const boost::system::error_code& error, boost::asio::ip::tcp::resolver::iterator endpoint_iter)
	{
		if(!error)
		{
			boost::asio::async_write(sock, req, boost::bind(&addonAsyncHTTP::writeHandle, this, boost::asio::placeholders::error, boost::asio::placeholders::bytes_transferred));
		}
		else if(endpoint_iter != boost::asio::ip::tcp::resolver::iterator())
		{
			sock.close();

			boost::asio::ip::tcp::endpoint endp = *endpoint_iter;
			sock.async_connect(endp, boost::bind(&addonAsyncHTTP::connectHandle, this, boost::asio::placeholders::error, ++endpoint_iter));
		}
		else
		{
			response.assign(error.message());
			response_code = error.value();
			// catch error
		}
	}

	void writeHandle(const boost::system::error_code& error, std::size_t bytes_tx)
	{
		if(!error)
		{
			boost::asio::async_read_until(sock, res, "\r\n", boost::bind(&addonAsyncHTTP::statusReadHandle, this, boost::asio::placeholders::error, boost::asio::placeholders::bytes_transferred));
		}
		else
		{
			response.assign(error.message());
			response_code = error.value();
			// catch error
		}
	}

	void statusReadHandle(const boost::system::error_code& error, std::size_t bytes_rx)
	{
		if(!error)
		{
			std::istream rsStream(&res);
			std::string http_ver;
			std::string status_msg;

			rsStream >> http_ver;
			rsStream >> response_code;
			std::getline(rsStream, response);

			if(!rsStream || http_ver.substr(0, 5) != "HTTP/")
			{
				response.assign("INVALID");
				response_code = 0;

				return;
			}

			boost::asio::async_read_until(sock, res, "\r\n\r\n", boost::bind(&addonAsyncHTTP::headerReadHandle, this, boost::asio::placeholders::error, boost::asio::placeholders::bytes_transferred));
		}
		else
		{
			response.assign(error.message());
			response_code = error.value();
			// catch error
		}
	}

	void headerReadHandle(const boost::system::error_code& error, std::size_t bytes_rx)
	{
		if(!error)
		{
			std::istream rsStream(&res);
			std::string header;

			while(std::getline(rsStream, header) && (header != "\r"))
				headers << header << std::endl;

			//if(res.size() > 0)
				//result << &res;

			boost::asio::async_read(sock, res, boost::asio::transfer_at_least(1), boost::bind(&addonAsyncHTTP::readHandle, this, boost::asio::placeholders::error, boost::asio::placeholders::bytes_transferred));
		}
		else
		{
			response.assign(error.message());
			response_code = error.value();
			// catch error
		}
	}

	void readHandle(const boost::system::error_code& error, std::size_t bytes_rx)
	{
		if(!error)
		{
			//result << &res;
			boost::asio::async_read(sock, res, boost::asio::transfer_at_least(1), boost::bind(&addonAsyncHTTP::readHandle, this, boost::asio::placeholders::error, boost::asio::placeholders::bytes_transferred));
		}
		else if(error == boost::asio::error::eof)
		{
			result_size = res.size();
			result = new char[result_size + 1];
			res.sgetn(result, result_size);

			readyFlag = true;
		}
		else
		{
			response.assign(error.message());
			response_code = error.value();
			// catch error
		}
	}

	bool isReady() const
	{
		return readyFlag;
	}

	std::string getResponse() const
	{
		return response;
	}

	int getResponseCode() const
	{
		return response_code;
	}

	std::string getHeaders() const
	{
		return headers.str();
	}

	char *getResult() const
	{
		return result;
	}

	std::size_t getResultSize() const
	{
		return result_size;
	}

private:

	boost::asio::io_service& io_s;
	boost::asio::ip::tcp::resolver resolr;
	boost::asio::ip::tcp::socket sock;
	boost::asio::streambuf req;
	boost::asio::streambuf res;

	boost::shared_ptr<boost::thread> asyncOPThread;

	std::string response;
	int response_code;

	std::stringstream headers;

	char *result;
	std::size_t result_size;

	bool readyFlag;
};