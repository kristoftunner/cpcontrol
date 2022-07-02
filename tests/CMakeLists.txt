cmake_minimum_required(VERSION 3.12)

add_executable(modbustcp_test
  modbustcp_test.cpp
)

target_link_libraries(modbustcp_test
  PUBLIC
  cpcontrol
  ${MODBUSPP_LIBRARIES}
)

target_include_directories(modbustcp_test
  PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}
)

add_executable(modbusrtu_test
  modbusrtu_test.cpp
)

target_link_libraries(modbusrtu_test
  PUBLIC
  cpcontrol
  ${MODBUSPP_LIBRARIES}
)

target_include_directories(modbusrtu_test
  PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}
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
