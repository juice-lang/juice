# src/juice/Basic/CMakeLists.txt - juice Basic sources CMake file
#
# This file is part of the juice open source project
#
# Copyright (c) 2019 - 2020 juice project authors
# Licensed under MIT License
#
# See https://github.com/juice-lang/juice/blob/master/LICENSE for license information
# See https://github.com/juice-lang/juice/blob/master/CONTRIBUTORS.txt for the list of juice project authors


add_library(juiceBasic STATIC
        Error.cpp
        Process.cpp
        RawStreamHelpers.cpp
        SourceBuffer.cpp
        SourceManager.cpp
        StringHelpers.cpp
        Version.cpp)

target_compile_options(juiceBasic PRIVATE ${LLVM_COMPILE_FLAG_LIST})


string(REGEX REPLACE "([0-9]+)\\.[0-9]+(\\.[0-9]+)?" "\\1" JUICE_VERSION_MAJOR ${JUICE_VERSION})
string(REGEX REPLACE "[0-9]+\\.([0-9]+)(\\.[0-9]+)?" "\\1" JUICE_VERSION_MINOR ${JUICE_VERSION})

target_compile_definitions(juiceBasic PRIVATE
        JUICE_VERSION=${JUICE_VERSION}
        JUICE_VERSION_MAJOR=${JUICE_VERSION_MAJOR}
        JUICE_VERSION_MINOR=${JUICE_VERSION_MINOR})

if("${JUICE_VERSION}" MATCHES "[0-9]+\\.[0-9]+\\.[0-9]+")
    string(REGEX REPLACE "[0-9]+\\.[0-9]+\\.([0-9]+)" "\\1" JUICE_VERSION_PATCHLEVEL
            ${JUICE_VERSION})
    target_compile_definitions(juiceBasic PRIVATE
            JUICE_HAS_VERSION_PATCHLEVEL=1
            JUICE_VERSION_PATCHLEVEL=${JUICE_VERSION_PATCHLEVEL})
else()
    target_compile_definitions(juiceBasic PRIVATE
            JUICE_HAS_VERSION_PATCHLEVEL=0)
endif()


if(DEFINED LLVM_PACKAGE_VERSION)
    target_compile_definitions(juiceBasic PRIVATE
            JUICE_HAS_LLVM_VERSION=1
            JUICE_LLVM_VERSION_MAJOR=${LLVM_VERSION_MAJOR}
            JUICE_LLVM_VERSION_MINOR=${LLVM_VERSION_MINOR}
            JUICE_LLVM_VERSION_PATCHLEVEL=${LLVM_VERSION_PATCH})
else()
    target_compile_definitions(juiceBasic PRIVATE
            JUICE_HAS_LLVM_VERSION=0)
endif()
