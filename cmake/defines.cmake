set_property(TARGET cpcontrol PROPERTY CMAKE_CXX_STANDARD 20)
set_property(TARGET cpcontrol PROPERTY CMAKE_CXX_STANDARD_REQUIRED ON)
message(STATUS "Compiler version:    
             ${CMAKE_CXX_COMPILER_VERSION}")
target_compile_options(cpcontrol
  PUBLIC
  -std=c++2a
  -ggdb
)
