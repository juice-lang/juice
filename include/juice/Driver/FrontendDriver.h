// include/juice/Driver/FrontendDriver.h - Driver subclass that does the actual compiler work
//
// This source file is part of the juice open source project
//
// Copyright (c) 2019 - 2021 juice project authors
// Licensed under MIT License
//
// See https://github.com/juice-lang/juice/blob/master/LICENSE for license information
// See https://github.com/juice-lang/juice/blob/master/CONTRIBUTORS.txt for the list of juice project authors


#ifndef JUICE_DRIVER_FRONTENDDRIVER_H
#define JUICE_DRIVER_FRONTENDDRIVER_H

#include "Driver.h"

#include "llvm/Support/CommandLine.h"

namespace juice {
    namespace driver {
        extern llvm::cl::SubCommand frontendSubcommand;

        class FrontendDriver: public Driver {
        public:
            FrontendDriver() = default;

            int execute() override;
        };
    }
}

#endif //JUICE_DRIVER_FRONTENDDRIVER_H
