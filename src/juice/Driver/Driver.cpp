// src/juice/Driver/Driver.cpp - Driver class
//
// This source file is part of the juice open source project
//
// Copyright (c) 2019 - 2020 juice project authors
// Licensed under MIT License
//
// See https://github.com/juice-lang/juice/blob/master/LICENSE for license information
// See https://github.com/juice-lang/juice/blob/master/CONTRIBUTORS.txt for the list of juice project authors


#include "juice/Driver/Driver.h"

#include "juice/Driver/FrontendDriver.h"
#include "juice/Driver/MainDriver.h"
#include "llvm/Support/CommandLine.h"

namespace juice {
    namespace driver {
        Driver * Driver::create(const char * firstArg) {
            Driver * driver = nullptr;

            for (auto * subcommand: llvm::cl::getRegisteredSubcommands()) {
                if (*subcommand) {
                    if (subcommand == &frontendSubcommand) {
                        driver = new FrontendDriver();
                    } else if (subcommand == &*llvm::cl::TopLevelSubCommand) {
                        driver = new MainDriver(firstArg);
                    } else {
                        llvm_unreachable("All subcommands should be handled here!");
                    }
                }
            }

            assert(driver != nullptr);

            return driver;
        }
    }
}
