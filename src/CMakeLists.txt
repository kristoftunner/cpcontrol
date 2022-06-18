cmake_minimum_required(VERSION 3.12)

find_package(modbuspp REQUIRED)
add_library(cpcontrol
  STATIC 
  device.cpp
  device_comm.cpp
  enerman.cpp
)

set_property(TARGET cpcontrol PROPERTY CMAKE_CXX_STANDARD 20)
set_property(TARGET cpcontrol PROPERTY CMAKE_CXX_STANDARD_REQUIRED ON)

target_include_directories(cpcontrol
  PUBLIC 
  ${CMAKE_CURRENT_SOURCE_DIR}
  ${CMAKE_CURRENT_SOURCE_DIR}/ext
)

target_link_libraries(cpcontrol
  PUBLIC modbuspp
)

add_executable(sandbox_app
  sandbox.cpp
)

target_link_libraries(sandbox_app
  PUBLIC
  cpcontrol
)
target_include_directories(sandbox_app
  PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}
)