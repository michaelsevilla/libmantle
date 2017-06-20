cmake_minimum_required(VERSION 2.8.9)
project(test)

# build the static library
include_directories(include)
set(SOURCES src/Mantle.cc)
add_library(mantle STATIC ${SOURCES})
#install(TARGETS mantle DESTINATION /usr/lib)

# build program that uses library
set(PROJECT_LINK_LIBS libmantle.a)
link_directories(build)
add_executable(test src/test.cc)
target_link_libraries(test ${PROJECT_LINK_LIBS})
