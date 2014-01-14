vpath %.cpp ../src
ifdef dbg
DEBUG:=-DDEBUG -g -ggdb3 -rdynamic
endif

NYNN_MM_INC= ../../../inc/

CXXFLAGS =--std=c++0x  ${DEBUG} -I $(NYNN_MM_INC) -lpthread

BIN=sg_write sg_read sg_readvtx

default:$(BIN) 
	mv -f $^ ../../../bin/bin

sg_write:sg_write.o
	g++ $(CXXFLAGS) -o $@ $^
sg_read: sg_read.o
	g++ $(CXXFLAGS) -o $@ $^
sg_readvtx: sg_readvtx.o
	g++ $(CXXFLAGS) -o $@ $^
%.o:%.cpp
	g++ $(CXXFLAGS) -c -o $@ $^
clean:
	$(RM) -r $(BIN)

