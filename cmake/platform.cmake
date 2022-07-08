set(CROSS_COMPILE "arm-linux-gnueabihf")
set(CROSS_COMPILE_CC "${CROSS_COMPILE}-gcc")
set(CROSS_COMPILE_CXX "${CROSS_COMPILE}-g++")
set(CROSS_COMPILE_LD "${CROSS_COMPILE}-ld")
set(CROSS_COMPILE_AR "${CROSS_COMPILE}-ar")
set(CROSS_COMPILE_RANLIB "${CROSS_COMPILE}-ranlib")
set(CROSS_COMPILE_STRIP "${CROSS_COMPILE}-strip")
set(CROSS_COMPILE_NM "${CROSS_COMPILE}-nm")
set(CROSS_COMPILE_OBJCOPY "${CROSS_COMPILE}-objcopy")
set(CROSS_COMPILE_SIZE "${CROSS_COMPILE}-size")

# At the very least, cross compile gcc and g++ have to be set!
find_program (CROSS_COMPILE_CC_FOUND ${CROSS_COMPILE_CC} REQUIRED)
find_program (CROSS_COMPILE_CXX_FOUND ${CROSS_COMPILE_CXX} REQUIRED)
set(CMAKE_SYSTEM_NAME "Linux")
set(CMAKE_SYSTEM_PROCESSOR "arm")
# Define the compiler
set(CMAKE_C_COMPILER ${CROSS_COMPILE_CC})
set(CMAKE_CXX_COMPILER ${CROSS_COMPILE_CXX})