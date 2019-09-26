# CMakeLists.txt - Main CMake file
#
# This file is part of the juice open source project
#
# Copyright (c) 2019 juice project authors
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

set(LLVM_INSTALL_DIR ${PROJECT_SOURCE_DIR}/../juice-llvm/build/install)
set(LLVM_LIB_DIR ${LLVM_INSTALL_DIR}/lib)

include_directories(${PROJECT_SOURCE_DIR}/include)
include_directories(${LLVM_INSTALL_DIR}/include)
add_subdirectory(src)
add_subdirectory(tools)
