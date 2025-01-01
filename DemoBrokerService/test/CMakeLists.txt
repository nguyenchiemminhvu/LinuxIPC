cmake_minimum_required(VERSION 3.10...3.28)
project(TestBroker)

set(CMAKE_BUILD_TYPE Debug)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED True)

file(GLOB SOURCES "*.cpp")

add_executable(TestBroker ${SOURCES})

target_include_directories(TestBroker PRIVATE ../core/)
