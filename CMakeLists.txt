cmake_minimum_required(VERSION 2.8.9)
project(test)

# setup MPI (courtesy of https://cmake.org/pipermail/cmake/2011-June/045037.html)
find_package(MPI REQUIRED)
include_directories(${MPI_INCLUDE_PATH})

# compile lua
add_subdirectory(src/lua)

# build the static library
include_directories(include)
set(SOURCES src/Mantle.cc)
add_library(mantle STATIC ${SOURCES})
target_link_libraries(mantle liblua)

# build program that uses library
link_directories(build)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -std=c++11")
add_executable(test src/test.cc)
target_link_libraries(test mantle liblua ${MPI_LIBRARIES})
