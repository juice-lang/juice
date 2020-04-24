# src/juice/Parser/CMakeLists.txt - juice Parser sources CMake file
#
# This file is part of the juice open source project
#
# Copyright (c) 2019 juice project authors
# Licensed under MIT License
#
# See https://github.com/juice-lang/juice/blob/master/LICENSE for license information
# See https://github.com/juice-lang/juice/blob/master/CONTRIBUTORS.txt for the list of juice project authors


add_compile_options(${LLVM_COMPILE_FLAGS})

add_library(juiceParser STATIC
        FSM.cpp
        Lexer.cpp
        LexerToken.cpp
        Parser.cpp)
