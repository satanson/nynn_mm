vpath %.cpp ../src
ifdef dbg
DEBUG:=-DDEBUG -g -ggdb3 -rdynamic
endif

ifdef read_without_abuse_copys 
READ_WITHOUT_ABUSE_COPYS:=-DREAD_WITHOUT_ABUSE_COPYS
endif

ifdef log
LOG:=-DLOG
endif
ifdef lockfree
LOCKFREE:=-DLOCKFREE
endif

ifdef sched
SCHED:=-DSCHED
endif

CXXFLAGS =-std=c++0x -L/usr/local/lib -lzmq -I../inc -I../../../inc -I/usr/include/c++/4.4.7/ $(DEBUG) $(READ_WITHOUT_ABUSE_COPYS) $(LOG) $(LOCKFREE) $(SCHED)

SRC = cli2_fillvtx.cpp cli2_read.cpp cli2_read_multithread.cpp cli2_readvtx.cpp read_single.cpp write_single.cpp test_write_single.cpp test_write_multiple.cpp test_write_single_16.cpp test_write_multiple_16.cpp test_rc_write.cpp test_rc_read.cpp test_read_single.cpp test_read_multiple.cpp test_read_vtxno.cpp 
		  
OBJ=$(addsuffix .o, $(basename $(SRC)))
BIN_DOT_A=$(addsuffix .a, $(basename $(SRC)))
BIN=$(basename $(SRC))


default:$(BIN_DOT_A)
	ls *.a|xargs -I{} echo '{}' |perl -ne 'chomp;$$i=$$_;s/\..*//g;rename $$i,$$_'
	mv -f $(BIN) ../../../bin/

%.a:%.o
	g++ $(CXXFLAGS) -o $@ $^ 
