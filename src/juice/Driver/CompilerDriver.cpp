// src/juice/Driver/CompilerDriver.cpp - Driver subclass for compiling a file
//
// This source file is part of the juice open source project
//
// Copyright (c) 2019 juice project authors
// Licensed under MIT License
//
// See https://github.com/juice-lang/juice/blob/master/LICENSE for license information
// See https://github.com/juice-lang/juice/blob/master/CONTRIBUTORS.txt for the list of juice project authors


#include "juice/Driver/CompilerDriver.h"

#include <iostream>

namespace juice {
    namespace driver {
        int CompilerDriver::execute() {
            std::cout << "Compile file " << _filename << std::endl; // TODO
            return 0;
        }
    }
}
