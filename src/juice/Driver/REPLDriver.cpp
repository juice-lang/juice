// src/juice/Driver/REPLDriver.cpp - Driver subclass for running the REPL
//
// This source file is part of the juice open source project
//
// Copyright (c) 2019 juice project authors
// Licensed under MIT License
//
// See https://github.com/juice-lang/juice/blob/master/LICENSE for license information
// See https://github.com/juice-lang/juice/blob/master/CONTRIBUTORS.txt for the list of juice project authors


#include "juice/Driver/REPLDriver.h"

#include <iostream>

namespace juice {
    namespace driver {
        int REPLDriver::execute() {
            std::cout << "REPL" << std::endl; // TODO
            return 0;
        }
    }
}
