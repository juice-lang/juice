# src/juice/IRGen/CMakeLists.txt - juice IRGen sources CMake file
#
# This file is part of the juice open source project
#
# Copyright (c) 2019 - 2020 juice project authors
# Licensed under MIT License
#
# See https://github.com/juice-lang/juice/blob/master/LICENSE for license information
# See https://github.com/juice-lang/juice/blob/master/CONTRIBUTORS.txt for the list of juice project authors


add_library(juiceIRGen STATIC
        GenDeclaration.cpp
        GenExpression.cpp
        GenStatement.cpp
        IRGen.cpp)

target_compile_options(juiceIRGen PRIVATE ${LLVM_COMPILE_FLAG_LIST})
