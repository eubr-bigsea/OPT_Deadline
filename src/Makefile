CXX=g++
CXXFLAGS=-std=c++14 -g -O0
OPT_COMMON_INCLUDE=/home/biagio/repositories/OPT_Common/include
EXE=opt_deadline

all: opt_deadline.o Process.o CoarseGrain.o FineGrain.o InitialSolution_FA.o InitialSolution_SA.o Algorithm1.o Algorithm2.o
	${CXX} ${CXXFLAGS} -o ${EXE} opt_deadline.o Process.o CoarseGrain.o FineGrain.o InitialSolution_FA.o InitialSolution_SA.o Algorithm1.o Algorithm2.o

opt_deadline.o: opt_deadline.cpp Process.hpp CoarseGrain.hpp
	${CXX} ${CXXFLAGS} -I${OPT_COMMON_INCLUDE} -c opt_deadline.cpp

Process.o: Process.cpp Process.hpp
	${CXX} ${CXXFLAGS} -I${OPT_COMMON_INCLUDE} -c Process.cpp

CoarseGrain.o: Process.hpp CoarseGrain.cpp CoarseGrain.hpp
	${CXX} ${CXXFLAGS} -I${OPT_COMMON_INCLUDE} -c CoarseGrain.cpp

FineGrain.o: FineGrain.hpp Process.hpp FineGrain.cpp
	${CXX} ${CXXFLAGS} -I${OPT_COMMON_INCLUDE} -c FineGrain.cpp

InitialSolution_FA.o: InitialSolution_FA.cpp InitialSolution_FA.hpp Process.hpp
	${CXX} ${CXXFLAGS} -I${OPT_COMMON_INCLUDE} -c InitialSolution_FA.cpp

InitialSolution_SA.o: InitialSolution_SA.cpp InitialSolution_SA.hpp Process.hpp
	${CXX} ${CXXFLAGS} -I${OPT_COMMON_INCLUDE} -c InitialSolution_SA.cpp

Algorithm1.o: Algorithm1.cpp Algorithm1.hpp FineGrain.hpp InitialSolution_FA.hpp
	${CXX} ${CXXFLAGS} -I${OPT_COMMON_INCLUDE} -c Algorithm1.cpp

Algorithm2.o: Algorithm2.cpp Algorithm2.hpp FineGrain.hpp InitialSolution_SA.hpp CoarseGrain.hpp
	${CXX} ${CXXFLAGS} -I${OPT_COMMON_INCLUDE} -c Algorithm2.cpp

clean:
	rm -f *.o
	rm -f ${EXE}
