cmake_minimum_required(VERSION 3.0)
project(ometacpp)
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_COMPILER g++-11)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -O0")
add_executable(test test.cpp)
