### -------------------- User Configurations --------------------    

# Specify the name of the directory moves cache, test, and train data
PYTHON_PACKAGE_NAME=GeneticShogi
DATA_DIR=$(shell (dirname `pwd`))/json
TRAIN_FILE=$(DATA_DIR)/train_data_5000.json
TEST_FILE=$(DATA_DIR)/test_data_5000.json
MOVES_FILE=$(DATA_DIR)/legal_moves_cache_10000.json

### ------------------- Build Configurations -------------------    

# Check system operating system and compiler availability
COMPILER := $(shell which g++-9)
UNAME := $(shell uname)

# Try to see if g++ version 9 available on system
ifeq (, $(COMPILER))
	@echo "Could not find g++ version 9, using default system g++"
	CXX=g++
else
	CXX=g++-9
endif

# Mac OSX specific flag required when building with pybind 11
ifeq ($(UNAME), Darwin)
	OS_FLAGS= -undefined dynamic_lookup
else
	OS_FLAGS=
endif

# Flags specific for building with pybind 11
PYBIND_FLAG= `python3 -m pybind11 --includes`
PYBIND_CONF= `python3-config --extension-suffix`

# Compiler flags to build shared library components and support multiprocessing
CXXFLAGS= -g3 -O3 -std=c++11 -fopenmp -fPIC

# Object file dependancies
DEPENDENCIES= train.o features.o lmcache.o helper.o shogi.o


### -------- Build Targets --------------###

# Default builds .so python3 package
all: $(PYTHON_PACKAGE_NAME)

$(PYTHON_PACKAGE_NAME):$(DEPENDENCIES) python3bind.o
	@echo "----- Creating Shared Library ---------"
	$(CXX) $(CXXFLAGS) $(OS_FLAGS) -shared -o $@$(PYBIND_CONF) $^
	@echo
	@echo "FINISHED"
	@echo "Succcessfully built python3 package: " $(PYTHON_PACKAGE_NAME)

# Target to run the main() function in train.cpp for debug/testing
train.test: $(DEPENDENCIES)
	@echo "-------- Creating Train Test -----------"
	$(CXX) $(CXXFLAGS) -o $@ $^
	@echo
	@echo "FINISHED"

### --------Object Files--------------###
python3bind.o: python3bind.cpp
	@echo "----- Building Python3 Binder  -------"
	$(CXX) $(CXXFLAGS) $(PYBIND_FLAG) -c $< -o $@
	@echo

shogi.o: shogi.cpp shogi.hpp
	@echo "----- Building Shogi Libraries -------"
	$(CXX) $(CXXFLAGS) $(PYBIND_FLAG) -c $< -o $@
	@echo

helper.o: helper.cpp helper.hpp
	@echo "----- Building Helper Functions ------"
	$(CXX) $(CXXFLAGS) $(PYBIND_FLAG) -c $< -o $@
	@echo

features.o: features.cpp features.hpp
	@echo "----- Building Features Evaluator -----"
	$(CXX) $(CXXFLAGS) $(PYBIND_FLAG) -c $< -o $@
	@echo

train.o: train.cpp
	@echo "----- Building Organism Evaluator -----"
	$(CXX) $(CXXFLAGS) $(PYBIND_FLAG) -c $< -o $@ \
		-D "TRAIN_FILE=$(TRAIN_FILE)" \
		-D "TEST_FILE=$(TEST_FILE)" \
		-D "MOVES_FILE=$(MOVES_FILE)"
	@echo

lmcache.o: lmcache.cpp lmcache.hpp
	@echo "----- Building MovesCache Wrapper ----"
	$(CXX) $(CXXFLAGS) $(PYBIND_FLAG) -c $< -o $@
	@echo

###--------CLEAN-UP--------------###
.PHONY: clean
clean:
	$(RM) -r *.test lmcache *.o *.gch *.dSYM *.so
