// include/juice/Driver/CompilerDriver.h - Driver subclass for compiling a file
//
// This source file is part of the juice open source project
//
// Copyright (c) 2019 - 2020 juice project authors
// Licensed under MIT License
//
// See https://github.com/juice-lang/juice/blob/master/LICENSE for license information
// See https://github.com/juice-lang/juice/blob/master/CONTRIBUTORS.txt for the list of juice project authors


#ifndef JUICE_DRIVER_COMPILERDRIVER_H
#define JUICE_DRIVER_COMPILERDRIVER_H

#include <string>
#include <utility>

#include "Driver.h"

namespace juice {
    namespace driver {
        class CompilerDriver: public Driver {
            std::string _filename;
        public:
            explicit CompilerDriver(std::string filename): _filename(std::move(filename)) {}

            int execute() override;
        };
    }
}

#endif //JUICE_DRIVER_COMPILERDRIVER_H
