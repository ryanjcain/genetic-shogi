### -------------------- User Configurations --------------------    

# Specify the name of the directory moves cache, test, and train data
DATA_DIR=$(shell (dirname `pwd`))/json
TRAIN_FILE=$(DATA_DIR)/train_data_raw.json
TEST_FILE=$(DATA_DIR)/test_data_raw.json
MOVES_FILE=$(DATA_DIR)/legal_moves_cache_raw.json

### ------------------- Target Configurations -------------------    
ifeq (, $(shell which g++-9))
	CXX=g++
else
	CXX=g++-9
endif
CXXFLAGS= -g3 -O3 -std=c++11 -fopenmp
CXXFLAGS_SO= -g3 -O3 -std=c++11 -fopenmp -fPIC -shared

all: shogilib.so

shogilib.so: CXXFLAGS= $(CXXFLAGS_SO)
shogilib.so: features.o lmcache.o helper.o shogi.o
	@echo "----- Creating Shared Library ---------"
	${CXX} ${CXXFLAGS} -o $@ $^
	@echo
	@echo "FINISHED"

features.test: features.o lmcache.o helper.o shogi.o
	@echo "----- Creating Feature Test -----------"
	${CXX} ${CXXFLAGS} -o $@ $^
	@echo
	@echo "FINISHED"

# Run to save a pretty version of cache/training data in /json 
lmcache-pretty: lmcache.cpp
	${CXX} ${CXXFLAGS} -o $@ $^

### --------Object Files--------------###

shogi.o: shogi.cpp shogi.hpp
	@echo "----- Building Shogi Libraries -------"
	${CXX} ${CXXFLAGS} -c $^
	@echo

helper.o: helper.cpp shogi.hpp
	@echo "----- Building Helper Functions ------"
	${CXX} ${CXXFLAGS} -c $^
	@echo

features.o: features.cpp features.hpp
	@echo "----- Building Features Evaluator -----"
	${CXX} ${CXXFLAGS} -c $^ \
		-D "TRAIN_FILE=$(TRAIN_FILE)" \
		-D "TEST_FILE=$(TEST_FILE)" \
		-D "MOVES_FILE=$(MOVES_FILE)"
	@echo

lmcache.o: lmcache.cpp lmcache.hpp
	@echo "----- Building MovesCache Wrapper ----"
	${CXX} ${CXXFLAGS} -c $^
	@echo

###--------CLEAN-UP--------------###
clean:
	$(RM) -r shogiTest featuresTest lmcache *.o *.gch *.dSYM *.so
