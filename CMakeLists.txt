# CMakeLists.txt - Main CMake file
#
# This file is part of the juice open source project
#
# Copyright (c) 2019 - 2020 juice project authors
# Licensed under MIT License
#
# See https://github.com/juice-lang/juice/blob/master/LICENSE for license information
# See https://github.com/juice-lang/juice/blob/master/CONTRIBUTORS.txt for the list of juice project authors


cmake_minimum_required(VERSION 3.1...3.14)

# For CMake versions before version 1.12, which don't understand the previous command
if(${CMAKE_VERSION} VERSION_LESS 3.12)
    cmake_policy(VERSION ${CMAKE_MAJOR_VERSION}.${CMAKE_MINOR_VERSION})
endif()

# Activate C++14
set(CMAKE_CXX_STANDARD 14)


project(juice DESCRIPTION "The juicy programming language" LANGUAGES CXX)

set(JUICE_VERSION "1.0")

set(LLVM_INSTALL_DIR "${PROJECT_SOURCE_DIR}/../juice-llvm/build/install")

set(LLVM_CONFIG_CMD "${LLVM_INSTALL_DIR}/bin/llvm-config")
execute_process(COMMAND ${LLVM_CONFIG_CMD} --cxxflags
        OUTPUT_VARIABLE LLVM_COMPILE_FLAGS
        OUTPUT_STRIP_TRAILING_WHITESPACE)

string(REPLACE
        "-std=c++11" "-std=c++14" LLVM_COMPILE_FLAGS "${LLVM_COMPILE_FLAGS}")
string(REGEX REPLACE
        "-fno-exceptions ?" "" OTHER_COMPILE_FLAGS "${LLVM_COMPILE_FLAGS}")

separate_arguments(LLVM_COMPILE_FLAGS)
separate_arguments(OTHER_COMPILE_FLAGS)

execute_process(COMMAND ${LLVM_CONFIG_CMD} --libfiles
        OUTPUT_VARIABLE LLVM_LIBS
        OUTPUT_STRIP_TRAILING_WHITESPACE)
separate_arguments(LLVM_LIBS)

add_definitions("-fvisibility=hidden")

include_directories(${PROJECT_SOURCE_DIR}/include)
include_directories(${LLVM_INSTALL_DIR}/include)
add_subdirectory(src)
add_subdirectory(tools)
