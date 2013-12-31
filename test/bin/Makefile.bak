vpath %.cc ../src 
ifdef dbg
DEBUG:=-DDEBUG -g -ggdb3 -rdynamic
endif
FLAGS:= -I../inc -I../../inc/ -lrt -lpthread -std=c++0x
progs:= formatSubgraph testAccessSubgraph testMultiAccessSubgraph \
        testSubgraphCache testSynchronization testMultiAccessCache \
		testBlock testAccessSubgraphSet

all:$(progs)

testAccessSubgraphSet:testAccessSubgraphSet.cc
	g++ $(DEBUG) $(FLAGS) -o $@ $^
testBlock:testBlock.cc
	g++ $(DEBUG) $(FLAGS) -o $@ $^
testMultiAccessCache:testMultiAccessCache.cc
	g++ $(DEBUG) $(FLAGS) -o $@ $^
formatSubgraph:formatSubgraph.cc
	g++ $(DEBUG) $(FLAGS) -o $@ $^
testSynchronization:testSynchronization.cc
	g++ $(DEBUG) $(FLAGS) -o $@ $^
testAccessSubgraph:testAccessSubgraph.cc
	g++ $(DEBUG) $(FLAGS) -o $@ $^
testMultiAccessSubgraph:testMultiAccessSubgraph.cc
	g++ $(DEBUG) $(FLAGS) -o $@ $^
testSubgraphCache:testSubgraphCache.cc
	g++ $(DEBUG) $(FLAGS) -o $@ $^


