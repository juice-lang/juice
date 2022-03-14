// include/juice/Driver/DriverTask.h - Task for the driver to execute
//
// This source file is part of the juice open source project
//
// Copyright (c) 2019 - 2021 juice project authors
// Licensed under MIT License
//
// See https://github.com/juice-lang/juice/blob/master/LICENSE for license information
// See https://github.com/juice-lang/juice/blob/master/CONTRIBUTORS.txt for the list of juice project authors


#ifndef JUICE_DRIVER_DRIVERTASK_H
#define JUICE_DRIVER_DRIVERTASK_H

#include <memory>
#include <string>
#include <system_error>

#include "DriverAction.h"
#include "juice/Diagnostics/Diagnostics.h"
#include "llvm/ADT/SmallString.h"
#include "llvm/ADT/SmallVector.h"
#include "llvm/ADT/StringRef.h"
#include "llvm/ADT/Twine.h"
#include "llvm/Support/Chrono.h"
#include "llvm/Support/Error.h"
#include "llvm/Option/Option.h"

namespace juice {
    namespace driver {
        class DriverTask {
        public:
            enum class Kind {
                input,
                compilation,
                linking
            };

        private:
            const Kind _kind;

        protected:
            std::string _executablePath;
            llvm::SmallVector<std::string, 16> _arguments;
            llvm::SmallVector<std::unique_ptr<DriverTask>, 4> _inputs;
            std::string _outputPath;
            bool _outputIsTemporary;

        public:
            DriverTask() = delete;

            DriverTask(Kind kind, std::string executablePath, llvm::SmallVector<std::string, 16> arguments,
                       llvm::SmallVectorImpl<std::unique_ptr<DriverTask>> && inputs, std::string outputPath,
                       bool outputIsTemporary);

            virtual llvm::Expected<bool> executeIfNecessary(llvm::sys::TimePoint<> timePoint);

            llvm::Error execute();


            llvm::StringRef getExecutablePathRef() const { return _executablePath; }
            llvm::ArrayRef<std::string> getArguments() const { return _arguments; }
            llvm::ArrayRef<std::unique_ptr<DriverTask>> getInputs() const { return _inputs; }
            const std::string & getOutputPath() const { return _outputPath; }
            llvm::StringRef getOutputPathRef() const { return _outputPath; }

        private:
            llvm::Expected<bool> executeInputs(llvm::sys::TimePoint<> timePoint);

        public:
            Kind getKind() const { return _kind; }
        };

        class InputTask: public DriverTask {
        public:
            InputTask() = delete;

            explicit InputTask(std::string inputPath);

            llvm::Expected<bool> executeIfNecessary(llvm::sys::TimePoint<> timePoint) override;

        public:
            static bool classof(const DriverTask * task) {
                return task->getKind() == Kind::input;
            }
        };

        class CompilationTask: public DriverTask {
            CompilationTask(std::string executablePath, llvm::SmallVector<std::string, 16> arguments,
                            llvm::SmallVectorImpl<std::unique_ptr<DriverTask>> && inputs,
                            std::string outputPath, bool outputIsTemporary);

        public:
            CompilationTask() = delete;

            static llvm::Expected<std::unique_ptr<CompilationTask>>
            create(const char * firstArg, DriverAction action, std::unique_ptr<InputTask> input);
            static std::unique_ptr<CompilationTask>
            create(const char * firstArg, DriverAction action, std::unique_ptr<InputTask> input, std::string outputPath,
                   bool outputIsTemporary = false);


            static bool classof(const DriverTask * task) {
                return task->getKind() == Kind::compilation;
            }
        };

        class LinkingTask: public DriverTask {
            LinkingTask(std::string executablePath, llvm::SmallVector<std::string, 16> arguments,
                        llvm::SmallVectorImpl<std::unique_ptr<DriverTask>> && inputs,
                        std::string outputPath);

        public:
            LinkingTask() = delete;

            static llvm::Expected<std::unique_ptr<LinkingTask>>
            create(llvm::SmallVectorImpl<std::unique_ptr<DriverTask>> && inputs, std::string outputPath);


            static bool classof(const DriverTask * task) {
                return task->getKind() == Kind::linking;
            }
        };
    }
}

#endif //JUICE_DRIVER_DRIVERTASK_H
