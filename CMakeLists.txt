cmake_minimum_required(VERSION 2.8.9)
project(test)

# compile lua
add_subdirectory(src/lua)

# build the static library
include_directories(include)
set(SOURCES src/Mantle.cc)
add_library(mantle STATIC ${SOURCES})
target_link_libraries(mantle liblua)
#install(TARGETS mantle DESTINATION /usr/lib)

# build program that uses library
#set(PROJECT_LINK_LIBS libmantle.a)
#set(PROJECT_LINK_LIBS liblua.a)
link_directories(build)
add_executable(test src/test.cc)
target_link_libraries(test mantle liblua)
