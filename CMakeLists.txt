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


set(LLVM_INSTALL_DIR "${PROJECT_SOURCE_DIR}/../juice-llvm/llvm/build/install")


find_package(LLVM REQUIRED CONFIG PATHS ${LLVM_INSTALL_DIR} NO_DEFAULT_PATH)

message(STATUS "Found LLVM ${LLVM_PACKAGE_VERSION}")
message(STATUS "Using LLVMConfig.cmake in: ${LLVM_DIR}")


set(LLVM_COMPILE_FLAG_LIST "-std=c++14" "-fvisibility=hidden")

if(NOT ${LLVM_ENABLE_RTTI})
    list(APPEND LLVM_COMPILE_FLAG_LIST "-fno-rtti")
endif()

if(NOT ${LLVM_ENABLE_EH})
    list(APPEND LLVM_COMPILE_FLAG_LIST "-fno-exceptions")
endif()


separate_arguments(LLVM_DEFINITIONS_LIST NATIVE_COMMAND ${LLVM_DEFINITIONS})
add_definitions(${LLVM_DEFINITIONS_LIST})


option(FORCE_COLORED_OUTPUT "Always produce ANSI-colored output (GNU/Clang only)." FALSE)
if(${FORCE_COLORED_OUTPUT})
    if("${CMAKE_CXX_COMPILER_ID}" STREQUAL "GNU")
        add_compile_options(-fdiagnostics-color=always)
    elseif("${CMAKE_CXX_COMPILER_ID}" MATCHES "Clang")
        add_compile_options(-fcolor-diagnostics)
    endif()
endif()


llvm_map_components_to_libnames(LLVM_LIB_LIST support core)

foreach(target ${LLVM_TARGETS_TO_BUILD})
    set(asm_parser "LLVM${target}AsmParser")
    set(code_gen "LLVM${target}CodeGen")
    if(${asm_parser} IN_LIST LLVM_AVAILABLE_LIBS)
        list(APPEND LLVM_LIB_LIST ${asm_parser})
    endif()
    if(${code_gen} IN_LIST LLVM_AVAILABLE_LIBS)
        list(APPEND LLVM_LIB_LIST ${code_gen})
    endif()
endforeach()


include_directories(${PROJECT_SOURCE_DIR}/include)
include_directories(${LLVM_INCLUDE_DIRS})
add_subdirectory(src)
add_subdirectory(tools)
