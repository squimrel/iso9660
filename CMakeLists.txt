cmake_minimum_required(VERSION 2.6)
project(iso9660)

set(CMAKE_CXX_FLAGS  "${CMAKE_CXX_FLAGS} -std=c++11 -Wall -O3")

include_directories(.)
add_library(iso9660 SHARED include/iso9660.h include/read.h include/write.h
  include/utility.h src/iso9660.cc src/read.cc src/utility.cc)

add_executable(persistent-storage example/persistent-storage.cc)
target_link_libraries(persistent-storage iso9660)
