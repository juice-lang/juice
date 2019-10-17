# src/juice/Basic/CMakeLists.txt - juice Basic sources CMake file
#
# This file is part of the juice open source project
#
# Copyright (c) 2019 juice project authors
# Licensed under MIT License
#
# See https://github.com/juice-lang/juice/blob/master/LICENSE for license information
# See https://github.com/juice-lang/juice/blob/master/CONTRIBUTORS.txt for the list of juice project authors


add_library(juiceBasic STATIC
        SourceBuffer.cpp
        SourceManager.cpp
        StringHelpers.cpp
        StringRef.cpp
        Version.cpp)

string(REGEX REPLACE "([0-9]+)\\.[0-9]+(\\.[0-9]+)?" "\\1" JUICE_VERSION_MAJOR ${JUICE_VERSION})
string(REGEX REPLACE "[0-9]+\\.([0-9]+)(\\.[0-9]+)?" "\\1" JUICE_VERSION_MINOR ${JUICE_VERSION})

set_property(SOURCE Version.cpp APPEND_STRING PROPERTY COMPILE_FLAGS
        " -DJUICE_VERSION=${JUICE_VERSION} -DJUICE_VERSION_MAJOR=${JUICE_VERSION_MAJOR} -DJUICE_VERSION_MINOR=${JUICE_VERSION_MINOR}")

if ("${JUICE_VERSION}" MATCHES "[0-9]+\\.[0-9]+\\.[0-9]+")
    string(REGEX REPLACE "[0-9]+\\.[0-9]+\\.([0-9]+)" "\\1" JUICE_VERSION_PATCHLEVEL
            ${JUICE_VERSION})
    set_property(SOURCE Version.cpp APPEND_STRING PROPERTY COMPILE_FLAGS
            " -DJUICE_HAS_VERSION_PATCHLEVEL=1 -DJUICE_VERSION_PATCHLEVEL=${JUICE_VERSION_PATCHLEVEL}")
else()
    set_property(SOURCE Version.cpp APPEND_STRING PROPERTY COMPILE_FLAGS
            " -DJUICE_HAS_VERSION_PATCHLEVEL=0")
endif()
