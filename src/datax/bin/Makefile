vpath %.cpp ../src
ifdef dbg
DEBUG:=-DDEBUG -g -ggdb3 -rdynamic
endif

CXXFLAGS =-std=c++0x -L/usr/local/lib -lzmq -I../inc -I../../../inc -I/usr/include/c++/4.4.7/ $(DEBUG)

SRC = nynn_mm_serv.cpp threadDemo.cpp test_nynn_log.cpp test_nynn_exception.cpp test_nynn_thread.cpp test_nynn_util.cpp getaddrinfoDemo.cpp
		  
OBJ=$(addsuffix .o, $(basename $(SRC)))
BIN=$(basename $(SRC))

default:$(BIN)

nynn_mm_serv:nynn_mm_serv.o
	g++ $(CXXFLAGS) -o $@ $^ 
getaddrinfoDemo:getaddrinfoDemo.o
	g++ $(CXXFLAGS) -o $@ $^ 
test_nynn_util:test_nynn_util.o
	g++ $(CXXFLAGS) -o $@ $^ 
test_nynn_exception:test_nynn_exception.o
	g++ $(CXXFLAGS) -o $@ $^ 
test_nynn_thread:test_nynn_thread.o
	g++ $(CXXFLAGS) -o $@ $^ 
test_nynn_log:test_nynn_log.o
	g++ $(CXXFLAGS) -o $@ $^ 
threadDemo:threadDemo.o
	g++ $(CXXFLAGS) -o $@ $^ 
%.o:%.cpp
	g++ $(CXXFLAGS) -c -o $@ $^

clean:
	$(RM) -rf $(BIN)
	$(RM) -rf *.o


