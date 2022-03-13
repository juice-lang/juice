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
#include "llvm/Support/Error.h"
#include "llvm/Support/raw_ostream.h"

namespace juice {
    namespace driver {
        extern llvm::cl::SubCommand frontendSubcommand;

        class FrontendDriver: public Driver {
            enum class Action: uint8_t {
                dumpParse,
                dumpAST,
                emitIR,
                emitObject
            };

            static llvm::cl::opt<std::string> inputFile;
            static llvm::cl::opt<std::string> outputFile;

            static llvm::cl::opt<Action> action;


            llvm::raw_pwrite_stream * _outputOS = nullptr;

        public:
            FrontendDriver() = default;

            ~FrontendDriver() override;

            int execute() override;

        private:
            llvm::Expected<llvm::raw_pwrite_stream &> getOutputOS();
        };
    }
}

#endif //JUICE_DRIVER_FRONTENDDRIVER_H
