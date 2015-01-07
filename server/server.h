#pragma once
#pragma warning(disable:4996)



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

//#include <boost/archive/text_iarchive.hpp>
//#include <boost/archive/text_oarchive.hpp>
#include <boost/asio.hpp>
#include <boost/atomic.hpp>
#include <boost/bind.hpp>
#include <boost/chrono.hpp>
#include <boost/filesystem.hpp>
//#include <boost/serialization/serialization.hpp>
#include <boost/shared_ptr.hpp>
#include <boost/thread.hpp>
#include <boost/unordered_map.hpp>

#define HAVE_STDINT_H

#include "../build/server/SDK/plugin.h"

#include "core.h"
#include "debug.h"
#include "natives.h"
#include "pool.h"
#include "string.h"
#include "tcpsocket.h"





typedef void (*logprintf_t)(char *format, ...);