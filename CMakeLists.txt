cmake_minimum_required(VERSION 3.12)

project(cpcontrol CXX)
include(cmake/platform.cmake)
include(cmake/defines.cmake)
add_subdirectory(src)
add_subdirectory(tests)
