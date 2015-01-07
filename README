What is it?
===========

It is client-server plugin, that adds functionality to SA:MP (GTA San Andreas Multiplayer)
To use that mod, you may install GTA:SA v1.0 US/EU and SA:MP 0.3a+ (properly works only with 0.3d+)


How do i use it?
================

At this moment, client is in alpha stage, when server is in deep-pre-alpha (coz server code is cross-platform)
If you want to test it, you can download all stuff you need from /build/server(client)/windows & /build/server(client)/linux folders in master branch


I want to build it myself!
==========================

Okay, if you want, you can compile it in VS2013 (solution file included) or using UNIX make (makefile included too)
All needle libraries & includes (excluding boost, ofc) placed in /build/include & /build/lib folders, for both Windows & Linux

Compiling on Windows (client, server & updater):
================================================

Download latest boost includes and place to your IDE include folder
Open solution file (addon.sln) and press F7. If all goes OK, all solution (client, server and updater) will built.

Compiling on Linux (server only):
=================================

Log in as root, then install/update your boost libraries (version 1.55.0 minimum required)

If you are using Debian/Ubuntu:
-------------------------------

	apt-get update
	apt-get install libboost-all-dev

Or, if you are using CentOS/RHEL:
---------------------------------

	yum update
	yum install boost-devel


When you updated boost libs, you can easily compile server binary by typing these commands:

	cd /path/to/server/source
	make
	
Wait for a 1-2 minutes... That's it! Compiled binary will be in /build/server/linux directory.

Russian topics:
---------------

http://forum.sa-mp.com/showthread.php?t=449479
http://pawno.su/showthread.php?t=97252
http://pawno-info.ru/showthread.php?t=162573
