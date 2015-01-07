GPP = g++
GCC = gcc

OUTFILE = "../build/server/linux/addon.so"

COMPILE_FLAGS = -c -O2 -fPIC -w -DLINUX -Wall -I../build/server/SDK/amx/
LIBS = ../build/lib/linux/boost_static/libboost_atomic.a ../build/lib/linux/boost_static/libboost_chrono.a ../build/lib/linux/boost_static/libboost_filesystem.a #../build/lib/linux/boost_static/libboost_system.a ../build/lib/linux/boost_static/libboost_thread.a

all:
	$(GCC) $(COMPILE_FLAGS) ../build/server/SDK/amx/*.c
	$(GPP) $(COMPILE_FLAGS) ../build/server/SDK/*.cpp
	$(GPP) $(COMPILE_FLAGS) *.cpp
	$(GPP) -O2 -fshort-wchar -shared -o $(OUTFILE) *.o $(LIBS)
	rm -f *.o
