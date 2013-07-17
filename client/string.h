#pragma once



#include "addon.h"





class addonString
{

public:

	std::string wstring_to_string(const std::wstring &input);
	std::string vprintf(const char *format, va_list args);
};



class formatString 
{

public:

    template<class T>
    formatString& operator<< (const T& arg) 
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