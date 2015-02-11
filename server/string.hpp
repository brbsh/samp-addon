#pragma once



#include "server.hpp"



#define UNIQUE_HEX_MOD \
	std::hex << std::uppercase << std::setw(8) << std::setfill('0')





namespace amxString
{
	void Set(AMX *amx, cell param, std::string data, std::size_t size);
	std::string Get(AMX *amx, cell param);
	std::string vprintf(const char *format, va_list args);
	bool isDecimial(const char *data, unsigned int size);
	bool isHexDecimial(const char *data, unsigned int size);
};



class strFormat 
{

public:

    template<class T>
    strFormat& operator<< (const T& arg) 
	{
        m_stream << arg;
        return *this;
    }
    operator std::string() const 
	{
        return m_stream.str();
    }

protected:

    std::stringstream m_stream;
};