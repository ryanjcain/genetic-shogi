CXX=g++-9
CXXFLAGS= -g3 -std=c++11 -fopenmp -fPIC -shared

all: shogilib.so

shogilib.so: features.o lmcache.o helper.o shogi.o
	${CXX} ${CXXFLAGS} -o $@ $^

featuresTest: features.o lmcache.o helper.o shogi.o
	${CXX} ${CXXFLAGS} -o $@ $^

# Run to save a pretty version of cache/training data in /json 
lmcache-pretty: lmcache.cpp
	${CXX} ${CXXFLAGS} -o $@ $^

### --------Object Files--------------###

shogi.o: shogi.cpp shogi.hpp
	${CXX} ${CXXFLAGS} -c $^

helper.o: helper.cpp shogi.hpp
	${CXX} ${CXXFLAGS} -c $^

features.o: features.cpp features.hpp #shogi.hpp shogi.cpp
	${CXX} ${CXXFLAGS} -c $^

lmcache.o: lmcache.cpp lmcache.hpp
	${CXX} ${CXXFLAGS} -c $^

###--------CLEAN-UP--------------###
clean:
	$(RM) -r shogiTest featuresTest lmcache *.o *.gch *.dSYM *.so