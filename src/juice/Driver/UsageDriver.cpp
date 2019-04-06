// src/juice/Driver/UsageDriver.cpp - Driver subclass for -h and --help arguments
//
// This source file is part of the juice open source project
//
// Copyright (c) 2019 juice project authors
// Licensed under MIT License
//
// See https://github.com/juice-lang/juice/blob/master/LICENSE for license information
// See https://github.com/juice-lang/juice/blob/master/CONTRIBUTORS.txt for the list of juice project authors


#include "juice/Driver/UsageDriver.h"

#include <iostream>

#include "juice/Basic/StringHelpers.h"

namespace juice {
    namespace driver {
        int UsageDriver::execute() {
            std::ostream & os = _error ? std::cerr : std::cout;

            os << "OVERVIEW: juice compiler" << std::endl << std::endl;
            os << "USAGE: juice [options] <inputs>" << std::endl << std::endl;
            os << "OPTIONS:" << std::endl;
            os << "  " << basic::resize("-h, --help", 20);
            os << "Display this message" << std::endl;
            os << "  " << basic::resize("-v, --version", 20);
            os << "Print version info and exit" << std::endl;

            return _error ? 1 : 0;
        }
    }
}
