#pragma once
#pragma comment(lib, "d3d9.lib")
#pragma comment(lib, "d3dx9.lib")
#pragma comment(lib, "urlmon.lib")
#pragma warning(disable:4996 4244)





#define WIN32_LEAN_AND_MEAN
#include <Windows.h> // Windows-only, alright?
//#include <WinSock2.h>



//#include <boost/archive/text_iarchive.hpp>
//#include <boost/archive/text_oarchive.hpp>
#include <boost/asio.hpp>
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
#include <d3d9.h>
#include <d3dx9.h>
#include <d3dx9tex.h>
#include <fstream>
#include <iostream>
#include <list>
#include <queue>
#include <sstream>
#include <string>



#include "ProxyIDirect3D9.hpp"
#include "ProxyIDirect3DDevice9.hpp"

#include "addrpool.hpp"
#include "core.hpp"
#include "d3device.hpp"
#include "debug.hpp"
#include "functions.hpp"
#include "loader.hpp"
#include "pool.hpp"
#include "string.hpp"
#include "tcpsocket.hpp"





typedef IDirect3D9 *(WINAPI *pfnDirect3DCreate9)(UINT SDKVersion);