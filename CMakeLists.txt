cmake_minimum_required(VERSION 3.18)
project(c64)
set(CMAKE_CXX_STANDARD 20)

add_definitions(-DFMT_HEADER_ONLY) # We have only included the header from fmt
find_package(Python3 COMPONENTS Interpreter Development REQUIRED)
find_package(pybind11 CONFIG REQUIRED)

include_directories(include/ external/ ${pybind11_INCLUDE_DIRS})

set(C64_LIBRARY_SOURCE
        src/cpu_6502.cpp
        src/c64.cpp
        src/instrumentation.cpp)

pybind11_add_module(pyc64 src/pyc64.cpp ${C64_LIBRARY_SOURCE})

add_library(c64 SHARED ${C64_LIBRARY_SOURCE})

set(ROM_DIR ${CMAKE_CURRENT_BINARY_DIR}/roms)

add_subdirectory(tests)