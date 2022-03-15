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

        __attribute__((unused)) llvm::cl::alias MainDriver::outputFilenameAlias(
            "output-file",
            llvm::cl::desc("Alias for -o"),
            llvm::cl::value_desc("file"),
            llvm::cl::aliasopt(outputFilename)
        );

        llvm::cl::opt<DriverAction::Kind> MainDriver::action(
            llvm::cl::desc("Choose action"),
            llvm::cl::values(
                clEnumValN(DriverAction::dumpParse, "dump-parse", "Parse input file and dump AST"),
                clEnumValN(DriverAction::dumpAST, "dump-ast", "Parse and type-check input file and dump AST"),
                clEnumValN(DriverAction::emitIR, "emit-ir", "Compile input file and emit generated LLVM IR"),
                clEnumValN(DriverAction::emitObject, "emit-object",
                           "Compile input file and emit generated object file"),
                clEnumValN(DriverAction::emitExecutable, "emit-exec",
                           "Compile input file and emit generated executable")
            ),
            llvm::cl::init(DriverAction::emitExecutable)
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

            auto outputFile = getAction().outputFile(inputFilename, outputFilename);

            if (outputFile.hasValue()) {
                if (action == DriverAction::emitExecutable) {
                    auto compilationTask = CompilationTask::create(_firstArg, getAction(), std::move(inputTask));
                    if (auto error = compilationTask.takeError())
                        return error;

                    llvm::SmallVector<std::unique_ptr<DriverTask>, 4> linkerInputs;
                    linkerInputs.push_back(std::move(*compilationTask));

                    return LinkingTask::create(std::move(linkerInputs), (std::string)outputFile.getValue());
                } else {
                    return CompilationTask::create(_firstArg, getAction(), std::move(inputTask),
                                                   (std::string)outputFile.getValue());
                }
            } else {
                if (action == DriverAction::emitExecutable) {
                    return basic::createError<diag::StaticDiagnosticError>(diag::DiagnosticID::linker_output_to_stdout);
                } else if (action == DriverAction::emitObject) {
                    return basic::createError<diag::StaticDiagnosticError>(diag::DiagnosticID::object_to_stdout);
                } else {
                    return CompilationTask::create(_firstArg, getAction(), std::move(inputTask), "-");
                }
            }
        }
    }
}
