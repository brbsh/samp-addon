#pragma once



#include "client.hpp"





namespace addonString
{
	std::string vprintf(const char *format, va_list args);
};



class strFormat
{

public:

    template<class T>
    strFormat& operator<<(const T& arg) 
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