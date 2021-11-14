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
#include "llvm/ADT/SmallString.h"
#include "llvm/Support/Path.h"
#include "llvm/Support/raw_ostream.h"

namespace juice {
    namespace driver {
        llvm::cl::opt<std::string> MainDriver::inputFilename(
            llvm::cl::Positional,
            llvm::cl::desc("<input file>"),
            llvm::cl::Required
        );

        llvm::cl::opt<std::string> MainDriver::outputFilename(
            "o",
            llvm::cl::desc("Write output to <file>"),
            llvm::cl::value_desc("file")
        );

        llvm::cl::alias MainDriver::outputFilenameAlias(
            "output-file",
            llvm::cl::desc("Alias for -o"),
            llvm::cl::value_desc("file"),
            llvm::cl::aliasopt(outputFilename)
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
            llvm::SmallString<128> outputFile(outputFilename);
            if (outputFile.empty()) {
                outputFile = inputFilename;
                llvm::sys::path::replace_extension(outputFile, "");
            }

            auto inputTask = std::make_unique<InputTask>(inputFilename);

            auto compilationTask = CompilationTask::create(_firstArg, std::move(inputTask));
            if (auto error = compilationTask.takeError())
                return std::move(error);


            llvm::SmallVector<std::unique_ptr<DriverTask>, 4> linkerInputs;
            linkerInputs.push_back(std::move(*compilationTask));

            return LinkingTask::create(std::move(linkerInputs), outputFile.c_str());
        }
    }
}
