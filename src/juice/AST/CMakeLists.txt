# src/juice/AST/CMakeLists.txt - juice AST sources CMake file
#
# This file is part of the juice open source project
#
# Copyright (c) 2019 - 2020 juice project authors
# Licensed under MIT License
#
# See https://github.com/juice-lang/juice/blob/master/LICENSE for license information
# See https://github.com/juice-lang/juice/blob/master/CONTRIBUTORS.txt for the list of juice project authors


add_compile_options(${LLVM_COMPILE_FLAG_LIST})

add_library(juiceAST STATIC
        AST.cpp
        DeclarationAST.cpp
        ExpressionAST.cpp
        StatementAST.cpp
        TypeRepr.cpp)
