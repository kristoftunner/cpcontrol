cmake_minimum_required(VERSION 3.12)

project(cpcontrol CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_FLAGS "-Wall -g -Wextra -pthread")

add_subdirectory(src)

