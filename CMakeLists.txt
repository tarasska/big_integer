cmake_minimum_required(VERSION 3.12)
project(big_integer)

set(CMAKE_CXX_STANDARD 11)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11 -lpthread")
add_executable(big_integer gtest/gtest.h gtest/gtest-all.cc gtest/gtest_main.cc big_integer.h big_integer.cpp big_integer_testing.cpp)