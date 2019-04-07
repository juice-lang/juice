// include/juice/Driver/REPLDriver.h - Driver subclass for running the REPL
//
// This source file is part of the juice open source project
//
// Copyright (c) 2019 juice project authors
// Licensed under MIT License
//
// See https://github.com/juice-lang/juice/blob/master/LICENSE for license information
// See https://github.com/juice-lang/juice/blob/master/CONTRIBUTORS.txt for the list of juice project authors


#ifndef JUICE_REPLDRIVER_H
#define JUICE_REPLDRIVER_H

#include "Driver.h"

namespace juice {
    namespace driver {
        class REPLDriver: public Driver {
        public:
            REPLDriver() = default;

            int execute() override;
        };
    }
}

#endif //JUICE_REPLDRIVER_H
