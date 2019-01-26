CXX := g++
CPPFLAGS := -O3 -Wall -std=c++14

PROGRAM = test-2level test-2level-assoc

all: $(PROGRAM)

CC_FILE_2LEVEL = ../sim/predictor_2level.cc
HDR_FILE_2LEVEL = ../sim/predictor_2level.h
SRC_2LEVEL := $(CC_FILE_2LEVEL) $(HDR_FILE_2LEVEL)

OBJECTS_TEST_2LEVEL = 2level-automaton-sature-test.o 2level-branch-history-register-test.o 2level-bptable-test.o 2level-predictor-test.o 2level-bhr-table-hash-test.o 
OBJECTS_TEST_2LEVEL_ASSOC = 2level-automaton-sature-test.o 2level-branch-history-register-test.o 2level-bptable-test.o 2level-predictor-test.o 2level-bhr-table-assoc-test.o 
OBJECTS_2LEVEL = predictor_2level.o
OBJECTS_2LEVEL_ASSOC = predictor_2level_assoc.o # OBJECTS = $(objects_tests) $(objects_2level)

predictor_2level.o: $(SRC_2LEVEL)
	$(CXX) $(CPPFLAGS) -c $(CC_FILE_2LEVEL)

# predictor_2level_assoc.o : CPPFLAGS += -DBHR_TABLE_ASSOC
predictor_2level_assoc.o: $(SRC_2LEVEL)
	$(CXX) $(CPPFLAGS) -c $(CC_FILE_2LEVEL) -o $@

test.o: test.cc
	$(CXX) $(CPPFLAGS) -c $^

test-2level: $(OBJECTS_TEST_2LEVEL) $(OBJECTS_2LEVEL) test.o
	$(CXX) $(CPPFLAGS) -o $@ $^

test-2level-assoc: $(OBJECTS_TEST_2LEVEL_ASSOC) $(OBJECTS_2LEVEL_ASSOC) test.o
	$(CXX) $(CPPFLAGS) -o $@ $^

clean-bin:
	rm -f $(PROGRAM)

clean:
	rm -f *.o $(PROGRAM)
