// src/juice/Driver/VersionPrinter.cpp - CommandLine Handler for -v and --version arguments
//
// This source file is part of the juice open source project
//
// Copyright (c) 2019 - 2020 juice project authors
// Licensed under MIT License
//
// See https://github.com/juice-lang/juice/blob/master/LICENSE for license information
// See https://github.com/juice-lang/juice/blob/master/CONTRIBUTORS.txt for the list of juice project authors


#include "juice/Driver/VersionPrinter.h"

#include "juice/Basic/Version.h"

namespace juice {
    namespace driver {
        void VersionPrinter::print(llvm::raw_ostream & os) {
            auto currentVersion = basic::Version::getCurrent();
            os << "The juice-lang compiler\nVersion: " << currentVersion;

            if (auto llvmVersion = basic::Version::getLLVM()) {
                os << " (using LLVM version " << llvmVersion << ")";
            }

            os << "\n";
        }
    }
}
