// src/juice/Driver/DriverTask.cpp - Task for the driver to execute
//
// This source file is part of the juice open source project
//
// Copyright (c) 2019 - 2021 juice project authors
// Licensed under MIT License
//
// See https://github.com/juice-lang/juice/blob/master/LICENSE for license information
// See https://github.com/juice-lang/juice/blob/master/CONTRIBUTORS.txt for the list of juice project authors


#include "juice/Driver/DriverTask.h"

#include <chrono>
#include <utility>

#include "juice/Basic/Error.h"
#include "juice/Basic/Process.h"
#include "juice/Diagnostics/DiagnosticError.h"
#include "juice/Platform/Macros.h"
#include "llvm/Support/FileSystem.h"
#include "llvm/Support/Path.h"
#include "llvm/Support/Program.h"

#if OS_MAC
#include "juice/Platform/MacOS/SDKPath.h"
#endif

namespace juice {
    namespace driver {
        DriverTask::DriverTask(Kind kind, std::string executablePath, llvm::SmallVector<std::string, 16> arguments,
                               llvm::SmallVectorImpl<std::unique_ptr<DriverTask>> && inputs,
                               std::string outputPath, bool outputIsTemporary):
            _kind(kind), _executablePath(std::move(executablePath)), _arguments(std::move(arguments)),
            _inputs(std::move(inputs)), _outputPath(std::move(outputPath)), _outputIsTemporary(outputIsTemporary) {}

        llvm::Expected<bool> DriverTask::executeIfNecessary(llvm::sys::TimePoint<> timePoint) {
            if (_outputPath == "-") {
                if (auto error = executeInputs(timePoint).takeError())
                    return std::move(error);
            } else {
                llvm::sys::fs::file_status status;

                if (auto errorCode = llvm::sys::fs::status(_outputPath, status)) {
                    if (errorCode != std::errc::no_such_file_or_directory)
                        return basic::createError<diag::StaticDiagnosticError>(diag::DiagnosticID::file_status_error,
                                                                               getOutputPathRef(),
                                                                               errorCode);

                    if (auto error = executeInputs(timePoint).takeError())
                        return std::move(error);
                } else if (!llvm::sys::fs::is_regular_file(status)) {
                    return basic::createError<diag::StaticDiagnosticError>(diag::DiagnosticID::file_not_regular,
                                                                           getOutputPathRef());
                } else if (!_outputIsTemporary) {
                    auto modificationTime = status.getLastModificationTime();

                    auto inputsWereExecuted = executeInputs(modificationTime);
                    if (auto error = inputsWereExecuted.takeError())
                        return std::move(error);

                    if (!inputsWereExecuted) {
                        return modificationTime < timePoint;
                    }
                } else { // output is temporary
                    auto inputsWereExecuted = executeInputs(timePoint);
                    if (auto error = inputsWereExecuted.takeError())
                        return std::move(error);

                    if (!inputsWereExecuted) {
                        llvm::sys::fs::remove(_outputPath);
                        return false;
                    }
                }
            }


            llvm::SmallVector<llvm::StringRef, 16> arguments = {
                _executablePath
            };

            for (const std::string & argument: _arguments)
                arguments.emplace_back(argument);

            int exitCode = llvm::sys::ExecuteAndWait(_executablePath, arguments);

            if (exitCode != 0) {
                return createExecutionError(exitCode);
            }

            return true;
        }

        llvm::Error DriverTask::createExecutionError(int exitCode) {
            return basic::createError<diag::StaticDiagnosticError>(diag::DiagnosticID::execution_failed,
                                                                   getExecutablePathRef(), exitCode);
        }

        llvm::Error DriverTask::execute() {
            auto wasExecuted = executeIfNecessary(std::chrono::system_clock::now());
            if (auto error = wasExecuted.takeError()) {
                return std::move(error);
            }

            return llvm::Error::success();
        }

        llvm::Expected<bool> DriverTask::executeInputs(llvm::sys::TimePoint<> timePoint) {
            bool someInputWasExecuted = false;

            for (auto & input: _inputs) {
                auto wasExecuted = input->executeIfNecessary(timePoint);
                if (auto error = wasExecuted.takeError())
                    return std::move(error);

                if (wasExecuted)
                    someInputWasExecuted = true;
            }

            return someInputWasExecuted;
        }

        InputTask::InputTask(std::string inputPath):
            DriverTask(Kind::input, "", {}, llvm::SmallVector<std::unique_ptr<DriverTask>, 4>(),
                       std::move(inputPath), false) {}

        llvm::Expected<bool> InputTask::executeIfNecessary(llvm::sys::TimePoint<> timePoint) {
            llvm::sys::fs::file_status status;

            if (auto errorCode = llvm::sys::fs::status(getOutputPathRef(), status)) {
                if (errorCode == std::errc::no_such_file_or_directory)
                    return basic::createError<diag::StaticDiagnosticError>(diag::DiagnosticID::file_not_found,
                                                                           getOutputPathRef());
                return basic::createError<diag::StaticDiagnosticError>(diag::DiagnosticID::file_status_error,
                                                                       getOutputPathRef(), errorCode);
            }

            if (!llvm::sys::fs::is_regular_file(status)) {
                return basic::createError<diag::StaticDiagnosticError>(diag::DiagnosticID::file_not_regular,
                                                                       getOutputPathRef());
            }

            return status.getLastModificationTime() > timePoint;
        }

        CompilationTask::CompilationTask(std::string executablePath, llvm::SmallVector<std::string, 16> arguments,
                                         llvm::SmallVectorImpl<std::unique_ptr<DriverTask>> && inputs,
                                         std::string outputPath, bool outputIsTemporary):
            DriverTask(Kind::compilation, std::move(executablePath), std::move(arguments), std::move(inputs),
                       std::move(outputPath), outputIsTemporary) {}

        llvm::Expected<std::unique_ptr<CompilationTask>>
        CompilationTask::create(const char * firstArg, DriverAction action, std::unique_ptr<InputTask> input) {
            llvm::StringRef inputBaseName = llvm::sys::path::stem(input->getOutputPathRef());

            llvm::SmallString<128> tempOutputPath;
            if (auto errorCode = llvm::sys::fs::createTemporaryFile(inputBaseName, "o", tempOutputPath)) {
                return basic::createError<diag::StaticDiagnosticError>(diag::DiagnosticID::error_creating_temporary,
                                                                       inputBaseName, errorCode);
            }

            return CompilationTask::create(firstArg, action, std::move(input), std::string(tempOutputPath), true);
        }

        std::unique_ptr<CompilationTask>
        CompilationTask::create(const char * firstArg, DriverAction action, std::unique_ptr<InputTask> input,
                                std::string outputPath, bool outputIsTemporary) {
            std::string executablePath = basic::getMainExecutablePath(firstArg);

            std::string actionString;
            switch (action) {
                case DriverAction::dumpParse:
                    actionString = "--dump-parse";
                    break;
                case DriverAction::dumpAST:
                    actionString = "--dump-ast";
                    break;
                case DriverAction::emitIR:
                    actionString = "--emit-ir";
                    break;
                case DriverAction::emitObject:
                case DriverAction::emitExecutable:
                    actionString = "--emit-object";
                    break;
            }

            llvm::SmallVector<std::string, 16> arguments = {
                "frontend",
                actionString,
                "--input-file",
                input->getOutputPath(),
                "--output-file",
                outputPath
            };

            llvm::SmallVector<std::unique_ptr<DriverTask>, 4> inputs;
            inputs.push_back(std::move(input));

            return std::unique_ptr<CompilationTask>(
                new CompilationTask(std::move(executablePath), std::move(arguments), std::move(inputs),
                                    std::move(outputPath), outputIsTemporary));
        }

        llvm::Error CompilationTask::createExecutionError(int exitCode) {
            return llvm::make_error<basic::AlreadyHandledError>();
        }

        LinkingTask::LinkingTask(std::string executablePath, llvm::SmallVector<std::string, 16> arguments,
                                 llvm::SmallVectorImpl<std::unique_ptr<DriverTask>> && inputs,
                                 std::string outputPath):
            DriverTask(Kind::linking, std::move(executablePath), std::move(arguments), std::move(inputs),
                       std::move(outputPath), false) {}

        llvm::Expected<std::unique_ptr<LinkingTask>>
        LinkingTask::create(llvm::SmallVectorImpl<std::unique_ptr<DriverTask>> && inputs,
                            std::string outputPath) {
            auto executablePath = llvm::sys::findProgramByName("ld");
            if (std::error_code errorCode = executablePath.getError()) {
                return basic::createError<diag::StaticDiagnosticError>(diag::DiagnosticID::error_finding_program,
                                                                       "ld", errorCode);
            }

            #if OS_MAC
            auto sdkPath = platform::macOS::getSDKPath();
            if (auto error = sdkPath.takeError()) {
                return std::move(error);
            }
            #endif

            // TODO: make this work for other OSs than macOS as well...
            llvm::SmallVector<std::string, 16> arguments = {
            #if OS_MAC
                "-syslibroot",
                *sdkPath,
                "-lSystem"
            #endif
            };

            for (const auto & input: inputs) {
                arguments.push_back(input->getOutputPath());
            }

            arguments.push_back("-o");
            arguments.push_back(outputPath);

            return std::unique_ptr<LinkingTask>(
                new LinkingTask(std::move(*executablePath), std::move(arguments), std::move(inputs),
                                std::move(outputPath)));
        }
    }
}
