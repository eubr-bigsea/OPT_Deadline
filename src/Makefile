CXX=g++
CXXFLAGS=-std=c++14 -g -O2
OPT_COMMON_INCLUDE=/home/biagio/repositories/OPT_Common/include

Process.o: Process.cpp Process.hpp
	${CXX} ${CXXFLAGS} -I${OPT_COMMON_INCLUDE} -c Process.cpp

CoarseGrain.o: Process.hpp CoarseGrain.cpp CoarseGrain.hpp
	${CXX} ${CXXFLAGS} -I${OPT_COMMON_INCLUDE} -c CoarseGrain.cpp

clean:
	rm -f *.o
