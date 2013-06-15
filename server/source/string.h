#pragma once
#pragma warning(disable:4700)



#include "addon.h"





class amxString
{

public:

	void Set(AMX *amx, cell param, std::string data);
	std::string Get(AMX *amx, cell param);
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