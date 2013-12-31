vpath %.cc ../src 
ifdef dbg
DEBUG:=-DDEBUG -g -ggdb3 -rdynamic
endif
FLAGS:= -I../inc -I../../inc/ -lrt -lpthread -std=c++0x
progs:= formatSubgraph testAccessSubgraph testMultiAccessSubgraph \
        testSubgraphCache testSynchronization testMultiAccessCache \
		testBlock testPushShift testUnshiftPop testUnshiftShift \
		testPushPop testInsertPrevRemove testInsertNextRemove \
		testSubgraphSet 

all:$(progs)

testGetTuple:testGetTuple.cc
	g++ $(DEBUG) $(FLAGS) -o $@ $^
testGraphTable0:testGraphTable0.cc
	g++ $(DEBUG) $(FLAGS) -o $@ $^
testGraphTable:testGraphTable.cc
	g++ $(DEBUG) $(FLAGS) -o $@ $^
testSubgraphSet:testSubgraphSet.cc
	g++ $(DEBUG) $(FLAGS) -o $@ $^
testInsertNextRemove:testInsertNextRemove.cc
	g++ $(DEBUG) $(FLAGS) -o $@ $^
testInsertPrevRemove:testInsertPrevRemove.cc
	g++ $(DEBUG) $(FLAGS) -o $@ $^
testUnshiftShift:testUnshiftShift.cc
	g++ $(DEBUG) $(FLAGS) -o $@ $^
testPushPop:testPushPop.cc
	g++ $(DEBUG) $(FLAGS) -o $@ $^
testUnshiftPop:testUnshiftPop.cc
	g++ $(DEBUG) $(FLAGS) -o $@ $^
testPushShift:testPushShift.cc
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


