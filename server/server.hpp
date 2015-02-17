#pragma once
#pragma warning(disable:4996 4700 4244)



#if defined(WIN32) || defined(_WIN32) || defined(WIN64)
#define WIN32_LEAN_AND_MEAN
#include <Windows.h>
#endif



//#include <boost/archive/text_iarchive.hpp>
//#include <boost/archive/text_oarchive.hpp>
#include <boost/asio.hpp>
#include <boost/algorithm/string.hpp>
//#include <boost/atomic.hpp>
#include <boost/bind.hpp>
#include <boost/chrono.hpp>
#include <boost/crc.hpp>
#include <boost/filesystem.hpp>
#include <boost/format.hpp>
#include <boost/regex.hpp>
//#include <boost/serialization/serialization.hpp>
#include <boost/shared_ptr.hpp>
#include <boost/thread.hpp>
#include <boost/unordered_map.hpp>
#include <boost/utility/binary.hpp>


#include <algorithm>
#include <assert.h>
#include <cstdlib>
#include <cstdarg>
#include <fstream>
#include <iostream>
#include <list>
#include <queue>
#include <sstream>
#include <stdlib.h>
#include <string>
#include <vector>

#define HAVE_STDINT_H

#include "../build/server/SDK/plugin.h"

#include "classes.hpp"
#include "core.hpp"
#include "debug.hpp"
#include "hash.hpp"
#include "natives.hpp"
#include "pool.hpp"
#include "string.hpp"
#include "tcpsocket.hpp"





typedef void (*logprintf_t)(char *format, ...);