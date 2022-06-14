cmake_minimum_required(VERSION 3.12)


add_library(cpcontrol
  STATIC 
  device.cpp
)

target_include_directories(cpcontrol
  PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}
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