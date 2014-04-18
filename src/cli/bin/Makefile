vpath %.cpp ../src
ifdef dbg
DEBUG:=-DDEBUG -g -ggdb3 -rdynamic
endif

ifdef read_without_abuse_copys 
READ_WITHOUT_ABUSE_COPYS:=-DREAD_WITHOUT_ABUSE_COPYS
endif

CXXFLAGS =-std=c++0x -L/usr/local/lib -lzmq -I../inc -I../../../inc -I/usr/include/c++/4.4.7/ $(DEBUG) $(READ_WITHOUT_ABUSE_COPYS)

SRC = cli_readvtx.cpp cli_fillvtx.cpp cli_clearvtx.cpp cli_unshiftvtx.cpp cli_shiftvtx.cpp cli_pushvtx.cpp cli_popvtx.cpp cli_read.cpp cli_framework.cpp cli_read_multithread.cpp
		  
OBJ=$(addsuffix .o, $(basename $(SRC)))
BIN_DOT_A=$(addsuffix .a, $(basename $(SRC)))
BIN=$(basename $(SRC))


default:$(BIN_DOT_A)
	ls *.a|xargs -I{} echo '{}' |perl -ne 'chomp;$$i=$$_;s/\..*//g;rename $$i,$$_'
	mv -f $(BIN) ../../../bin/

cli_read_multithread:cli_read_multithread.o
	g++ $(CXXFLAGS) -o $@ $^ 

%.a:%.o
	g++ $(CXXFLAGS) -o $@ $^ 
