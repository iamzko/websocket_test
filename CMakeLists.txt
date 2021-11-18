cmake_minimum_required(VERSION 3.20)
project(websocket_test)

set(CMAKE_CXX_STANDARD 14)
link_libraries(pthread)

add_executable(websocket_test main.cpp)
