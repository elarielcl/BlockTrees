
cmake_minimum_required(VERSION 3.0)
project(BlockTrees)

set(CMAKE_CXX_STANDARD 11)
SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -O9")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11 -fopenmp")


include_directories(includes)
include_directories(external/sdsl-lite/include)

add_subdirectory(src)
#add_subdirectory(tests)
add_subdirectory(executables)

