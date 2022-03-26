cmake_minimum_required(VERSION 3.19)
project(plain_sockets)

set(CMAKE_CXX_STANDARD 17)

add_executable(plain_sockets
        threaded.cpp)
