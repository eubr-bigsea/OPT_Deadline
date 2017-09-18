CXX=g++

#Debug Flags
CXXFLAGS=-std=c++14 -g -O0 -Wall -Wextra -Wpedantic

#Release Flags
# CXXFLAGS=-std=c++14 -O3 -DNDEBUG

# OPT_Common Framework include directory
OPT_COMMON_INCLUDE=/home/biagio/repositories/OPT_Common/include

all: opt_deadline

opt_deadline:
	make -C src "CXX=${CXX}" "CXXFLAGS=${CXXFLAGS}" "OPT_COMMON_INCLUDE=${OPT_COMMON_INCLUDE}"
	cp src/opt_deadline .

clean:
	make clean -C src
	rm -f opt_deadline
