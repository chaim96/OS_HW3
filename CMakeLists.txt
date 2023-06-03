cmake_minimum_required(VERSION 3.25)
project(HW3 C)

set(CMAKE_C_STANDARD 11)
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -pthread")


add_executable(HW3 client.c output.c request.c request.h segel.c segel.h server.c output.c)
