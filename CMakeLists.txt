cmake_minimum_required(VERSION 2.6)
project(iso9660)

set(CMAKE_CXX_FLAGS  "${CMAKE_CXX_FLAGS} -std=c++11 -Wall -O3 -DNDEBUG")

include_directories(.)
file(GLOB FILES src/*.cc include/*.h)
add_library(iso9660 SHARED ${FILES})

add_executable(persistent-storage example/persistent-storage.cc)
target_link_libraries(persistent-storage iso9660)
