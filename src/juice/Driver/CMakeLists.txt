# src/juice/Driver/CMakeLists.txt - juice Driver sources CMake file
#
# This file is part of the juice open source project
#
# Copyright (c) 2019 - 2020 juice project authors
# Licensed under MIT License
#
# See https://github.com/juice-lang/juice/blob/master/LICENSE for license information
# See https://github.com/juice-lang/juice/blob/master/CONTRIBUTORS.txt for the list of juice project authors


add_compile_options(${OTHER_COMPILE_FLAG_LIST})

add_library(juiceDriver STATIC
        CompilerDriver.cpp
        Driver.cpp
        ErrorDriver.cpp
        REPLDriver.cpp
        UsageDriver.cpp
        VersionDriver.cpp)
