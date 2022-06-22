cmake_minimum_required(VERSION 3.12)

find_package(modbuspp REQUIRED)

add_library(cpcontrol
  STATIC 
  device.cpp
  device_comm.cpp
  enerman.cpp
  catchpenny.cpp
)

target_include_directories(cpcontrol
  PUBLIC 
  ${CMAKE_CURRENT_SOURCE_DIR}
  ${CMAKE_CURRENT_SOURCE_DIR}/ext
)

target_link_libraries(cpcontrol
  PUBLIC 
  modbuspp
  ${MODBUSPP_LIBRARIES}
)

add_executable(sandbox_app
  sandbox.cpp
)

target_link_libraries(sandbox_app
  PUBLIC
  cpcontrol
  ${MODBUSPP_LIBRARIES}
)
target_include_directories(sandbox_app
  PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}
)