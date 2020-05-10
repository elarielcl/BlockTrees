
cmake_minimum_required(VERSION 3.0)
project(BlockTree)

set(CMAKE_CXX_STANDARD 11)
set(SOURCE_FILES main.cpp)
SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -O9")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11 -fopenmp")

add_executable(main ${SOURCE_FILES})
add_executable(padding_main padding_main.cpp)
add_executable(inputs inputs.cpp)
add_executable(experiments_block_tree experiments_block_tree.cpp)
add_executable(experiments_padding_block_tree experiments_padding_block_tree.cpp)











include_directories(includes)
include_directories(external/sdsl-lite/include)

add_subdirectory(src)
#add_subdirectory(tests)

target_link_libraries(main blocktree)
target_link_libraries(main ${CMAKE_SOURCE_DIR}/external/sdsl-lite/lib/libsdsl.a)
target_link_libraries(main ${CMAKE_SOURCE_DIR}/external/sdsl-lite/lib/libdivsufsort.a)
target_link_libraries(main ${CMAKE_SOURCE_DIR}/external/sdsl-lite/lib/libdivsufsort64.a)


target_link_libraries(padding_main blocktree)
target_link_libraries(padding_main ${CMAKE_SOURCE_DIR}/external/sdsl-lite/lib/libsdsl.a)
target_link_libraries(padding_main ${CMAKE_SOURCE_DIR}/external/sdsl-lite/lib/libdivsufsort.a)
target_link_libraries(padding_main ${CMAKE_SOURCE_DIR}/external/sdsl-lite/lib/libdivsufsort64.a)


target_link_libraries(inputs ${CMAKE_SOURCE_DIR}/external/sdsl-lite/lib/libsdsl.a)
target_link_libraries(inputs ${CMAKE_SOURCE_DIR}/external/sdsl-lite/lib/libdivsufsort.a)
target_link_libraries(inputs ${CMAKE_SOURCE_DIR}/external/sdsl-lite/lib/libdivsufsort64.a)

target_link_libraries(experiments_block_tree blocktree)
target_link_libraries(experiments_block_tree ${CMAKE_SOURCE_DIR}/external/sdsl-lite/lib/libsdsl.a)
target_link_libraries(experiments_block_tree ${CMAKE_SOURCE_DIR}/external/sdsl-lite/lib/libdivsufsort.a)
target_link_libraries(experiments_block_tree ${CMAKE_SOURCE_DIR}/external/sdsl-lite/lib/libdivsufsort64.a)


target_link_libraries(experiments_padding_block_tree blocktree)
target_link_libraries(experiments_padding_block_tree ${CMAKE_SOURCE_DIR}/external/sdsl-lite/lib/libsdsl.a)
target_link_libraries(experiments_padding_block_tree ${CMAKE_SOURCE_DIR}/external/sdsl-lite/lib/libdivsufsort.a)
target_link_libraries(experiments_padding_block_tree ${CMAKE_SOURCE_DIR}/external/sdsl-lite/lib/libdivsufsort64.a)

