// src/juice/Driver/MainDriver.cpp - The main Driver class which decodes the given arguments into subprocess calls
//
// This source file is part of the juice open source project
//
// Copyright (c) 2019 - 2021 juice project authors
// Licensed under MIT License
//
// See https://github.com/juice-lang/juice/blob/master/LICENSE for license information
// See https://github.com/juice-lang/juice/blob/master/CONTRIBUTORS.txt for the list of juice project authors


#include "juice/Driver/MainDriver.h"

#include <utility>

#include "juice/Basic/Error.h"
#include "juice/Diagnostics/DiagnosticError.h"
#include "llvm/Support/raw_ostream.h"

namespace juice {
    namespace driver {
        llvm::cl::opt<std::string> MainDriver::inputFilename(
            llvm::cl::Positional,
            llvm::cl::desc("<input file>"),
            llvm::cl::Required
        );


        MainDriver::MainDriver(const char * firstArg): _firstArg(firstArg) {}

        int MainDriver::execute() {
            auto task = parseOptions();
            if (basic::handleAllErrors(task.takeError(), [](const diag::StaticDiagnosticError & error) {
                error.diagnose();
            })) {
                return 1;
            }

            if (basic::handleAllErrors(task.get()->execute(), [](const diag::StaticDiagnosticError & error) {
                error.diagnose();
            })) {
                return 1;
            }

            return 0;
        }

        llvm::Expected<std::unique_ptr<DriverTask>> MainDriver::parseOptions() {
            auto inputTask = std::make_unique<InputTask>(inputFilename);

            return CompilationTask::create(_firstArg, std::move(inputTask)); // No output path for now
        }
    }
}
