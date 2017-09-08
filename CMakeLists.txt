cmake_minimum_required(VERSION 3.0)

set(PROJECT_NAME opt_deadline)
project(${PROJECT_NAME})

set(PROJECT_SRC
  src/Algorithm1.cpp
  src/CoarseGrain.cpp
  src/InitialSolution_FA.cpp
  src/opt_deadline.cpp
  src/Algorithm2.cpp
  src/FineGrain.cpp
  src/InitialSolution_SA.cpp
  src/Process.cpp)

set(PROJECT_HEADERS
  src/Algorithm1.hpp
  src/CoarseGrain.hpp
  src/InitialSolution_FA.hpp
  src/Process.hpp
  src/Algorithm2.hpp
  src/FineGrain.hpp
  src/InitialSolution_SA.hpp)

# Require include OPT_Common
set(OPT_COMMON_DIR "" CACHE PATH "The include path of OPT_Common framework")

# Enable cxx std14
set(CMAKE_CXX_STANDARD 14)

add_executable(${PROJECT_NAME} ${PROJECT_SRC} ${PROJECT_HEADERS})
target_include_directories(${PROJECT_NAME} PUBLIC ${OPT_COMMON_DIR})

