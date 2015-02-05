#pragma once
#pragma warning(disable:4996 4700 4244)



#if defined(WIN32) || defined(_WIN32) || defined(WIN64)
#define WIN32_LEAN_AND_MEAN
#include <Windows.h>
#endif



//#include <boost/archive/text_iarchive.hpp>
//#include <boost/archive/text_oarchive.hpp>
#include <boost/asio.hpp>
//#include <boost/atomic.hpp>
#include <boost/bind.hpp>
#include <boost/chrono.hpp>
#include <boost/crc.hpp>
#include <boost/filesystem.hpp>
//#include <boost/serialization/serialization.hpp>
#include <boost/shared_ptr.hpp>
#include <boost/thread.hpp>
#include <boost/unordered_map.hpp>


#include <algorithm>
#include <assert.h>
#include <cstdlib>
#include <cstdarg>
#include <fstream>
#include <iostream>
#include <list>
#include <queue>
#include <sstream>
#include <string>

#define HAVE_STDINT_H

#include "../build/server/SDK/plugin.h"

#include "classes.h"
#include "core.h"
#include "debug.h"
#include "natives.h"
#include "pool.h"
#include "string.h"
#include "tcpsocket.h"





typedef void (*logprintf_t)(char *format, ...);