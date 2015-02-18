GPP = g++
GCC = gcc

OUTFILE = "../build/server/linux/addon.so"

COMPILE_FLAGS = -c -O2 -fPIC -w -DLINUX -Wall -I../build/server/SDK/amx/
LIBS = -lboost_chrono -lboost_filesystem -lboost_regex -lboost_system -lboost_thread

all:
	$(GCC) $(COMPILE_FLAGS) ../build/server/SDK/amx/*.c
	$(GPP) $(COMPILE_FLAGS) ../build/server/SDK/*.cpp
	$(GPP) $(COMPILE_FLAGS) *.cpp
	$(GPP) -O2 -fshort-wchar -shared -o $(OUTFILE) *.o $(LIBS)
	rm -f *.o
