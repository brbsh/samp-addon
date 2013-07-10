GPP = g++
GCC = gcc

OUTFILE = "../compiled/server/addon.so"

COMPILE_FLAGS = -c -O2 -fPIC -w -DLINUX -Wall -I./SDK/amx/

all:
	$(GCC) $(COMPILE_FLAGS) ./SDK/amx/*.c
	$(GPP) $(COMPILE_FLAGS) ./SDK/*.cpp
	$(GPP) $(COMPILE_FLAGS) *.cpp
	$(GPP) -O2 -fshort-wchar -shared -o $(OUTFILE) *.o ./lib/boost/libboost_thread.a
	rm -f *.o
