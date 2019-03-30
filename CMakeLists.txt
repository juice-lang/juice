# CMakeLists.txt - Main CMake file
#
# This file is part of the juice open source project
#
# Copyright (c) 2019 juice project authors
# Licensed under MIT License
#
# See https://github.com/juice-lang/juice/blob/master/LICENSE for license information
# See https://github.com/juice-lang/juice/blob/master/CONTRIBUTORS for the list of Swift project authors


cmake_minimum_required(VERSION 3.1...3.14)

# For CMake versions before version 1.12, which don't understand the previous command
if(${CMAKE_VERSION} VERSION_LESS 3.12)
    cmake_policy(VERSION ${CMAKE_MAJOR_VERSION}.${CMAKE_MINOR_VERSION})
endif()

# Activate C++11
set (CMAKE_CXX_STANDARD 11)


project(juice DESCRIPTION "The juicy programming language" LANGUAGES CXX)

set(JUICE_VERSION "1.0")

include_directories(${PROJECT_SOURCE_DIR}/include)
add_subdirectory(src)
add_subdirectory(tools)
