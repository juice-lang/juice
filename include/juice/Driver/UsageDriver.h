// include/juice/Driver/UsageDriver.h - Driver subclass for -h and --help arguments
//
// This source file is part of the juice open source project
//
// Copyright (c) 2019 - 2020 juice project authors
// Licensed under MIT License
//
// See https://github.com/juice-lang/juice/blob/master/LICENSE for license information
// See https://github.com/juice-lang/juice/blob/master/CONTRIBUTORS.txt for the list of juice project authors


#ifndef JUICE_DRIVER_USAGEDRIVER_H
#define JUICE_DRIVER_USAGEDRIVER_H

#include "Driver.h"

namespace juice {
    namespace driver {
        class UsageDriver: public Driver {
            bool _error;
        public:
            explicit UsageDriver(bool error): _error(error) {}

            int execute() override;
        };
    }
}

#endif //JUICE_DRIVER_USAGEDRIVER_H
