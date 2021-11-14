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
#include "llvm/Support/FileSystem.h"
#include "llvm/Support/Path.h"
#include "llvm/Support/Program.h"

namespace juice {
    namespace driver {
        DriverTask::DriverTask(Kind kind, std::string executablePath, llvm::SmallVector<llvm::StringRef, 16> arguments,
                               llvm::SmallVectorImpl<std::unique_ptr<DriverTask>> && inputs,
                               std::string outputPath, bool outputIsTemporary):
            _kind(kind), _executablePath(std::move(executablePath)), _arguments(std::move(arguments)),
            _inputs(std::move(inputs)), _outputPath(std::move(outputPath)), _outputIsTemporary(outputIsTemporary) {
            _arguments.insert(_arguments.begin(), executablePath);
        }

        llvm::Expected<bool> DriverTask::executeIfNecessary(llvm::sys::TimePoint<> timePoint) {
            llvm::sys::fs::file_status status;

            if (auto errorCode = llvm::sys::fs::status(_outputPath, status)) {
                if (errorCode != std::errc::no_such_file_or_directory)
                    return basic::createError<diag::StaticDiagnosticError>(diag::DiagnosticID::file_status_error,
                                                                           getOutputPath(), errorCode);
            } else if (!llvm::sys::fs::is_regular_file(status)) {
                return basic::createError<diag::StaticDiagnosticError>(diag::DiagnosticID::file_not_regular,
                                                                       getOutputPath());
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

            int result = llvm::sys::ExecuteAndWait(_executablePath, _arguments);

            if (result != 0) {
                return basic::createError<diag::StaticDiagnosticError>(diag::DiagnosticID::error_executing,
                                                                       getExecutablePath());
            }

            return true;
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

            if (auto errorCode = llvm::sys::fs::status(_outputPath, status)) {
                if (errorCode == std::errc::no_such_file_or_directory)
                    return basic::createError<diag::StaticDiagnosticError>(diag::DiagnosticID::file_not_found,
                                                                           getOutputPath());
                return basic::createError<diag::StaticDiagnosticError>(diag::DiagnosticID::file_status_error,
                                                                       getOutputPath(), errorCode);
            }

            if (!llvm::sys::fs::is_regular_file(status)) {
                return basic::createError<diag::StaticDiagnosticError>(diag::DiagnosticID::file_not_regular,
                                                                       getOutputPath());
            }

            return status.getLastModificationTime() > timePoint;
        }

        CompilationTask::CompilationTask(std::string executablePath, llvm::SmallVector<llvm::StringRef, 16> arguments,
                                         llvm::SmallVectorImpl<std::unique_ptr<DriverTask>> && inputs,
                                         std::string outputPath, bool outputIsTemporary):
            DriverTask(Kind::compilation, std::move(executablePath), std::move(arguments), std::move(inputs),
                       std::move(outputPath), outputIsTemporary) {}

        llvm::Expected<std::unique_ptr<CompilationTask>>
        CompilationTask::create(const char * firstArg, std::unique_ptr<InputTask> input) {
            llvm::StringRef inputBaseName = llvm::sys::path::stem(input->getOutputPath());

            llvm::SmallString<128> tempOutputPath;
            if (auto errorCode = llvm::sys::fs::createTemporaryFile(inputBaseName, "o", tempOutputPath)) {
                return basic::createError<diag::StaticDiagnosticError>(diag::DiagnosticID::error_creating_temporary,
                                                                       inputBaseName, errorCode);
            }

            return CompilationTask::create(firstArg, std::move(input), std::string(tempOutputPath), true);
        }

        std::unique_ptr<CompilationTask>
        CompilationTask::create(const char * firstArg, std::unique_ptr<InputTask> input,
                                std::string outputPath, bool outputIsTemporary) {
            std::string executablePath = basic::getMainExecutablePath(firstArg);

            llvm::SmallVector<llvm::StringRef, 16> arguments = {
                "frontend",
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

        LinkingTask::LinkingTask(std::string executablePath, llvm::SmallVector<llvm::StringRef, 16> arguments,
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

            // TODO: make this work for other computers than my own as well...
            llvm::SmallVector<llvm::StringRef, 16> arguments = {
                "-syslibroot",
                "/Applications/Xcode.app/Contents/Developer/Platforms/MacOSX.platform/Developer/SDKs/MacOSX.sdk",
                "-lSystem"
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
