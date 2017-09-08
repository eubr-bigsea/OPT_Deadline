# OPT_Deadline Project

## Requirements
The project has been developed for *NIX systems.
Tested on Linux with GCC compiler.

* C++14 support is required.
* OPT_Common framework. You can found here ([OPT_Framework Repository](https://github.com/BiagioFesta/OPT_Common)).
* DagSim Project. ([DagSim Framework](https://github.com/eubr-bigsea/dagSim))

## Installation

1. Obtain source files cloning repository:
~~~
git clone https://github.com/BiagioFesta/OPT_Deadline
~~~

2. Compiling with Make UNIX utility:
~~~
cd OPT_Deadline  && \
make OPT_COMMON_INCLUDE=/path/OPT_Common/include
~~~

**Alternatively**: you can compile with CMake utility (minimum verison required 3.0.1):
~~~
cd OPT_Deadline  && \
mkdir build && \
cd build && \
cmake .. -DOPT_COMMON_DIR=/path/OPT_Common/include -DCMAKE_BUILD_TYPE=Release && \
make
~~~

Note that your have to specify the include path of the framework library *OPT_Common*.

## Launch OPT_Deadline

You can launch OPT_Deadline from command line just typing:
~~~
./opt_deadline  PROCESS_FILE  CONFIG_FILE   (-1|-2|-12)
~~~

* `-1` specifies the algorithm 1.
* `-2` specifies the algorithm 2.
* `-12` will execute both algorithms.
