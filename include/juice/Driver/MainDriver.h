// include/juice/Driver/MainDriver.h - The main Driver class which decodes the given arguments into subprocess calls
//
// This source file is part of the juice open source project
//
// Copyright (c) 2019 - 2021 juice project authors
// Licensed under MIT License
//
// See https://github.com/juice-lang/juice/blob/master/LICENSE for license information
// See https://github.com/juice-lang/juice/blob/master/CONTRIBUTORS.txt for the list of juice project authors


#ifndef JUICE_DRIVER_MAINDRIVER_H
#define JUICE_DRIVER_MAINDRIVER_H

#include "Driver.h"

#include <memory>
#include <string>

#include "DriverAction.h"
#include "DriverTask.h"
#include "llvm/Support/CommandLine.h"
#include "llvm/Support/Error.h"

namespace juice {
    namespace driver {
        class MainDriver: public Driver {
            static llvm::cl::opt<std::string> inputFilename;
            static llvm::cl::opt<std::string> outputFilename;
            static llvm::cl::alias outputFilenameAlias;

            static llvm::cl::opt<DriverAction::Kind> action;

            static DriverAction getAction() { return action.getValue(); }



            const char * _firstArg;

        public:
            MainDriver() = delete;

            MainDriver(const char * firstArg);

            int execute() override;

        private:
            llvm::Expected<std::unique_ptr<DriverTask>> parseOptions();
        };
    }
}

#endif //JUICE_DRIVER_MAINDRIVER_H
