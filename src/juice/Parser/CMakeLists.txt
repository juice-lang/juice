# src/juice/Parser/CMakeLists.txt - juice Parser sources CMake file
#
# This file is part of the juice open source project
#
# Copyright (c) 2019 - 2020 juice project authors
# Licensed under MIT License
#
# See https://github.com/juice-lang/juice/blob/master/LICENSE for license information
# See https://github.com/juice-lang/juice/blob/master/CONTRIBUTORS.txt for the list of juice project authors


add_library(juiceParser STATIC
        FSM.cpp
        Lexer.cpp
        LexerToken.cpp
        Parser.cpp)

target_compile_options(juiceParser PRIVATE ${LLVM_COMPILE_FLAG_LIST})
