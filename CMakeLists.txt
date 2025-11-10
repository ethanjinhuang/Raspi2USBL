cmake_minimum_required(VERSION 3.6.0)

# 64 bit cross compiling
# SET(CMAKE_SYSTEM_NAME Linux)
# SET(CMAKE_SYSTEM_PROCESSOR arm)
# SET(CMAKE_C_COMPILER /opt/aarch64/bin/aarch64-linux-gnu-gcc)
# SET(CMAKE_CXX_COMPILER /opt/aarch64/bin/aarch64-linux-gnu-g++)

project(RaspiUSBL VERSION 0.1.0 LANGUAGES C CXX)

# set(CMAKE_CXX_STANDARD 11)
# set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_STANDARD_REQUIRED True)

# Using incremental compilation
find_program(CCACHE_PROGRAM ccache)
if(CCACHE_PROGRAM)
    set(CMAKE_C_COMPILER_LAUNCHER ${CCACHE_PROGRAM})
    set(CMAKE_CXX_COMPILER_LAUNCHER ${CCACHE_PROGRAM})
endif()

include(CTest)
enable_testing()

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")
INCLUDE_DIRECTORIES(${LOCAL_LIBRARY_DIRS})

# set gdb debug
set(CMAKE_BUILD_TYPE "Debug")
set(CMAKE_CXX_FLAGS_DEBUG "$ENV{CXXFLAGS} -O0 -Wall -g -ggdb")
set(CMAKE_CXX_FLAGS_RELEASE "$ENV{CXXFLAGS} -O3 -Wall")

set(CPACK_PROJECT_NAME ${PROJECT_NAME})
set(CPACK_PROJECT_VERSION ${PROJECT_VERSION})
include(CPack)

set(EXECUTABLE_OUTPUT_PATH ${PROJECT_SOURCE_DIR}/bin)
set(LIBRARY_OUTPUT_PATH ${PROJECT_SOURCE_DIR}/lib)

# Project
file(GLOB CONFIG_RESOURSES "config/*.h" "config/*.cpp")
file(GLOB GENERAL_RESOURSES "general/*.h" "general/*.cpp")
file(GLOB CORE_RESOURSES "core/*.h" "core/*.cpp")
file(GLOB DSP_RESOURSES "dsp/*.h" "dsp/*.cpp")
file(GLOB DAQ_RESOURSES "daq/*.h" "daq/*.cpp")
file(GLOB DAQAI_RESOURSES "daq/ai/*.h" "daq/ai/*.cpp")
file(GLOB DAQAO_RESOURSES "daq/ao/*.h" "daq/ao/*.cpp")
file(GLOB DATAIO_RESOURSES "dataio/*.h" "dataio/*.cpp")
file(GLOB FILEIO_RESOURSES "fileio/*.h" "fileio/*.cpp")
file(GLOB TOOL_RESOURSES "tool/*.h" "tool/*.cpp" "tool/*.hpp")

add_executable(RaspiUSBL
    ${CONFIG_RESOURSES}
    ${GENERAL_RESOURSES}
    ${CORE_RESOURSES}
    ${DSP_RESOURSES}
    ${DAQ_RESOURSES}
    ${DAQAI_RESOURSES}
    ${DAQAO_RESOURSES}
    ${FILEIO_RESOURSES}
    ${DATAIO_RESOURSES}
    ${TOOL_RESOURSES}
    main.cpp)

# include uldaq lib
target_link_libraries(RaspiUSBL uldaq)

# fftw3
add_subdirectory(thirdparty/fftw-3.3.10)
include_directories(${PROJECT_SOURCE_DIR}/thirdparty/fftw-3.3.10/api)
target_link_libraries(${PROJECT_NAME} fftw3)

# Eigen3
include_directories(thirdparty/eigen-3.4.0)
# ignore the Eigen-NEON warning
add_compile_options(-Wno-class-memaccess)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wno-class-memaccess")

# yaml
add_subdirectory(thirdparty/yaml-cpp-0.8.0)
target_link_libraries(${PROJECT_NAME} yaml-cpp)

# abseil
set(ABSL_PROPAGATE_CXX_STD true)
add_subdirectory(thirdparty/abseil-cpp)
target_link_libraries(${PROJECT_NAME} absl::strings absl::str_format absl::time)

# zlib
find_package(ZLIB REQUIRED)
include_directories(${ZLIB_INCLUDE_DIRS})
target_link_libraries(${PROJECT_NAME} ${ZLIB_LIBRARIES})

# liquid-dsp
# set(LIQUID_DSP_ROOT_DIR "${CMAKE_SOURCE_DIR}/thirdparty/liquid-dsp") # 设置库的路径
# include_directories("${LIQUID_DSP_ROOT_DIR}/include") # 设置头文件路径
# link_directories("${LIQUID_DSP_ROOT_DIR}") # 链接库文件
# target_link_libraries(${PROJECT_NAME} "${LIQUID_DSP_ROOT_DIR}/libliquid.so")