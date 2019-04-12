// include/juice/Driver/ErrorDriver.h - Driver subclass for argument parsing errors
//
// This source file is part of the juice open source project
//
// Copyright (c) 2019 juice project authors
// Licensed under MIT License
//
// See https://github.com/juice-lang/juice/blob/master/LICENSE for license information
// See https://github.com/juice-lang/juice/blob/master/CONTRIBUTORS.txt for the list of juice project authors


#ifndef JUICE_ERRORDRIVER_H
#define JUICE_ERRORDRIVER_H

#include <string>
#include <utility>

#include "Driver.h"

namespace juice {
    namespace driver {
        class ErrorDriver: public Driver {
            std::string _message;
        public:
            explicit ErrorDriver(std::string message): _message(std::move(message)) {}

            int execute() override;
        };
    }
}

#endif //JUICE_ERRORDRIVER_H
