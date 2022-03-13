// src/juice/Driver/DriverAction.cpp - The main action that the driver should perform
//
// This source file is part of the juice open source project
//
// Copyright (c) 2019 - 2022 juice project authors
// Licensed under MIT License
//
// See https://github.com/juice-lang/juice/blob/master/LICENSE for license information
// See https://github.com/juice-lang/juice/blob/master/CONTRIBUTORS.txt for the list of juice project authors


#include "juice/Driver/DriverAction.h"

#include "llvm/Support/Path.h"

namespace juice {
    namespace driver {
        llvm::Optional<llvm::SmallString<128>>
        DriverAction::outputFile(llvm::StringRef inputFilename, llvm::StringRef outputFilename) const {
            if (!outputFilename.empty()) {
                if (outputFilename == "-") {
                    return llvm::None;
                } else {
                    return {outputFilename};
                }
            }

            llvm::StringRef extension;
            switch (_kind) {
                case dumpParse:
                case dumpAST:
                case emitIR:
                    return llvm::None;
                case emitObject:
                    extension = "o";
                    break;
                case emitExecutable:
                    extension = "";
                    break;
            }

            llvm::SmallString<128> outputFile(inputFilename);
            llvm::sys::path::replace_extension(outputFile, extension);

            return outputFile;
        }
    }
}
