// include/juice/Driver/VersionDriver.h - Driver subclass for -v and --version arguments
//
// This source file is part of the juice open source project
//
// Copyright (c) 2019 juice project authors
// Licensed under MIT License
//
// See https://github.com/juice-lang/juice/blob/master/LICENSE for license information
// See https://github.com/juice-lang/juice/blob/master/CONTRIBUTORS.txt for the list of juice project authors


#ifndef JUICE_VERSIONDRIVER_H
#define JUICE_VERSIONDRIVER_H

#include "Driver.h"

namespace juice {
    namespace driver {
        class VersionDriver: public Driver {
        public:
            VersionDriver() = default;

            int execute() override;
        };
    }
}

#endif //JUICE_VERSIONDRIVER_H