// src/juice/Driver/ErrorDriver.cpp - Driver subclass for argument parsing errors
//
// This source file is part of the juice open source project
//
// Copyright (c) 2019 juice project authors
// Licensed under MIT License
//
// See https://github.com/juice-lang/juice/blob/master/LICENSE for license information
// See https://github.com/juice-lang/juice/blob/master/CONTRIBUTORS.txt for the list of juice project authors


#include "juice/Driver/ErrorDriver.h"

#include <iostream>

namespace juice {
    namespace driver {
        int ErrorDriver::execute() {
            std::cerr << "juice: error: " << _message << std::endl;
            return 1;
        }
    }
}
