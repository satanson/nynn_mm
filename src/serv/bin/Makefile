vpath %.cpp ../src
ifdef dbg
DEBUG:=-DDEBUG -g -ggdb3 -rdynamic
endif

CXXFLAGS =-std=c++0x -L/usr/local/lib -lzmq -I../inc -I../../../inc -I/usr/include/c++/4.4.7/ $(DEBUG)

SRC = nynn_mm_nameserv.cpp nynn_mm_dataserv.cpp
		  
OBJ=$(addsuffix .o, $(basename $(SRC)))
BIN=$(basename $(SRC))

default:nynn_mm_dataserv nynn_mm_nameserv
	mv -f $^ ../../../bin/bin

nynn_mm_dataserv:nynn_mm_dataserv.o
	g++ $(CXXFLAGS) -o $@ $^ 
nynn_mm_nameserv:nynn_mm_nameserv.o
	g++ $(CXXFLAGS) -o $@ $^ 
