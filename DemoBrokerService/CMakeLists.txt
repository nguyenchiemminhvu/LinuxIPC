cmake_minimum_required(VERSION 3.10...3.28)
project(BrokerServiceDemo)

set(CMAKE_BUILD_TYPE Debug)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED True)

add_subdirectory(core)
add_subdirectory(diagnostic_service)
add_subdirectory(location_service)
add_subdirectory(test)
