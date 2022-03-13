# tools/juice/CMakeLists.txt - juice tool CMake file
#
# This file is part of the juice open source project
#
# Copyright (c) 2019 - 2020 juice project authors
# Licensed under MIT License
#
# See https://github.com/juice-lang/juice/blob/master/LICENSE for license information
# See https://github.com/juice-lang/juice/blob/master/CONTRIBUTORS.txt for the list of juice project authors


add_executable(juice juice.cpp)

target_link_libraries(juice
        juiceAST
        juiceBasic
        juiceDiagnostics
        juiceDriver
        juiceIRGen
        juiceParser
        juiceSema)
target_link_libraries(juice ${LLVM_LIB_LIST})

target_compile_options(juice PRIVATE ${LLVM_COMPILE_FLAG_LIST})


install(TARGETS juice DESTINATION bin)
