CXX=g++
CXXFLAGS=-std=c++14 -g -O2
OPT_COMMON_INCLUDE=/home/biagio/repositories/OPT_Common/include
EXE=opt_deadline

all: opt_deadline.o Process.o CoarseGrain.o
	${CXX} ${CXXFLAGS} -o ${EXE} opt_deadline.o Process.o CoarseGrain.o

opt_deadline.o: opt_deadline.cpp Process.hpp CoarseGrain.hpp
	${CXX} ${CXXFLAGS} -I${OPT_COMMON_INCLUDE} -c opt_deadline.cpp

Process.o: Process.cpp Process.hpp
	${CXX} ${CXXFLAGS} -I${OPT_COMMON_INCLUDE} -c Process.cpp

CoarseGrain.o: Process.hpp CoarseGrain.cpp CoarseGrain.hpp
	${CXX} ${CXXFLAGS} -I${OPT_COMMON_INCLUDE} -c CoarseGrain.cpp

clean:
	rm -f *.o
	rm -f ${EXE}
