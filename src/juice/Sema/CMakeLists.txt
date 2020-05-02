# src/juice/Sema/CMakeLists.txt - juice Sema sources CMake file
#
# This file is part of the juice open source project
#
# Copyright (c) 2019 - 2020 juice project authors
# Licensed under MIT License
#
# See https://github.com/juice-lang/juice/blob/master/LICENSE for license information
# See https://github.com/juice-lang/juice/blob/master/CONTRIBUTORS.txt for the list of juice project authors


add_compile_options(${LLVM_COMPILE_FLAGS})

add_library(juiceSema STATIC
        BuiltinType.cpp
        Type.cpp
        TypeCheckedAST.cpp
        TypeCheckedDeclarationAST.cpp
        TypeCheckedExpressionAST.cpp
        TypeCheckedStatementAST.cpp
        TypeChecker.cpp)
