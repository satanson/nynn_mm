vpath %.cpp ../src
ifdef dbg
DEBUG:=-DDEBUG -g -ggdb3 -rdynamic
endif
ifdef read_without_abuse_copys 
READ_WITHOUT_ABUSE_COPYS:=-DREAD_WITHOUT_ABUSE_COPYS
endif

CXXFLAGS =-std=c++0x -L/usr/local/lib -lzmq -I../inc -I../../../inc -I/usr/include/c++/4.4.7/ $(DEBUG) $(READ_WITHOUT_ABUSE_COPYS)

SRC = cli_fillvtx2.cpp cli_readvtx2.cpp cli_read2.cpp cli_read_multithread2.cpp
		  
OBJ=$(addsuffix .o, $(basename $(SRC)))
BIN_DOT_A=$(addsuffix .a, $(basename $(SRC)))
BIN=$(basename $(SRC))


default:$(BIN_DOT_A)
	ls *.a|xargs -I{} echo '{}' |perl -ne 'chomp;$$i=$$_;s/\..*//g;rename $$i,$$_'
	mv -f $(BIN) ../../../bin/

%.a:%.o
	g++ $(CXXFLAGS) -o $@ $^ 
