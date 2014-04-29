vpath %.cpp ../src
ifdef dbg
DEBUG:=-DDEBUG -g -ggdb3 -rdynamic
endif
ifdef read_without_abuse_copys 
READ_WITHOUT_ABUSE_COPYS:=-DREAD_WITHOUT_ABUSE_COPYS
endif

CXXFLAGS =-std=c++0x -L/usr/local/lib -lzmq -I../inc -I../../../inc -I/usr/include/c++/4.4.7/ $(DEBUG) $(READ_WITHOUT_ABUSE_COPYS)

SRC = nynn_mm_nameserv.cpp nynn_mm_dataserv.cpp framework.cpp
		  
OBJ=$(addsuffix .o, $(basename $(SRC)))
BIN_DOT_A=$(addsuffix .a, $(basename $(SRC)))
BIN=$(basename $(SRC))


default:$(BIN_DOT_A)
	ls *.a|xargs -I{} echo '{}' |perl -ne 'chomp;$$i=$$_;s/\..*//g;rename $$i,$$_'
	mv -f $(BIN) ../../../bin/

%.a:%.o
	g++ $(CXXFLAGS) -o $@ $^ 
