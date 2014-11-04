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

SRC = sg_tidyvtx.cpp sg_remove_edges2.cpp sg_insert_edges2.cpp sg_insert_edges.cpp sg_dump_edges.cpp sg_modify_edges.cpp sg_format.cpp sg_write.cpp sg_read.cpp sg_readvtx.cpp sg_read_multithread.cpp test_read.cpp test_read1.cpp test_write_single1.cpp zms_client.cpp sg_write_multithread.cpp
		  
OBJ=$(addsuffix .o, $(basename $(SRC)))
BIN_DOT_A=$(addsuffix .a, $(basename $(SRC)))
BIN=$(basename $(SRC))


default:$(BIN_DOT_A)
	ls *.a|xargs -I{} echo '{}' |perl -ne 'chomp;$$i=$$_;s/\..*//g;rename $$i,$$_'
	mv -f $(BIN) ../../../bin/
	rm -fr *.o

%.a:%.o
	g++ $(CXXFLAGS) -o $@ $^ 
