#pragma once
#pragma comment(lib, "d3d9.lib")
#pragma comment(lib, "d3dx9.lib")
#pragma comment(lib, "urlmon.lib")
#pragma warning(disable:4996)



#define ADDON_STRING_VERSION	"2013.11.07"
#define ADDON_NUMERIC_VERSION	(20131107)



#include <algorithm>
#include <d3d9.h>
#include <d3dx9.h>
#include <d3dx9tex.h>
#include <fstream>
#include <iostream>
#include <list>
#include <queue>
#include <string>
#include <sstream>

#define WIN32_LEAN_AND_MEAN
#include <Windows.h> // Windows-only, alright?

#include <boost/asio.hpp>
#include <boost/bind.hpp>
#include <boost/chrono.hpp>
#include <boost/filesystem.hpp>
#include <boost/shared_ptr.hpp>
#include <boost/thread.hpp>
#include <boost/unordered_map.hpp>

#include "ProxyIDirect3D9.h"
#include "ProxyIDirect3DDevice9.h"

#include "addrpool.h"
#include "core.h"
#include "d3device.h"
#include "debug.h"
#include "functions.h"
#include "loader.h"
#include "string.h"
#include "tcpsocket.h"





typedef IDirect3D9 *(WINAPI *pfnDirect3DCreate9)(UINT SDKVersion);