# tools/juice/CMakeLists.txt - juice tool CMake file
#
# This file is part of the juice open source project
#
# Copyright (c) 2019 juice project authors
# Licensed under MIT License
#
# See https://github.com/juice-lang/juice/blob/master/LICENSE for license information
# See https://github.com/juice-lang/juice/blob/master/CONTRIBUTORS.txt for the list of juice project authors


add_executable(juice juice.cpp)
target_link_libraries(juice juiceBasic juiceDiagnostics juiceDriver)

install(TARGETS juice DESTINATION bin)
