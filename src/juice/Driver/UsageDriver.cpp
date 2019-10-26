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

#include "juice/Basic/StringHelpers.h"
#include "llvm/Support/raw_ostream.h"

namespace juice {
    namespace driver {
        int UsageDriver::execute() {
            llvm::raw_ostream & os = _error ? llvm::errs() : llvm::outs();

            os << "OVERVIEW: juice compiler\n\n";
            os << "USAGE: juice [options] <inputs>\n\n";
            os << "OPTIONS:\n";
            os << "  " << basic::resize("-h, --help", 20);
            os << "Display this message\n";
            os << "  " << basic::resize("-v, --version", 20);
            os << "Print version info and exit\n";

            return _error ? 1 : 0;
        }
    }
}
