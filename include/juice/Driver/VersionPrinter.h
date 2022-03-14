// include/juice/Driver/VersionPrinter.h - CommandLine Handler for -v and --version arguments
//
// This source file is part of the juice open source project
//
// Copyright (c) 2019 - 2020 juice project authors
// Licensed under MIT License
//
// See https://github.com/juice-lang/juice/blob/master/LICENSE for license information
// See https://github.com/juice-lang/juice/blob/master/CONTRIBUTORS.txt for the list of juice project authors


#ifndef JUICE_DRIVER_VERSIONPRINTER_H
#define JUICE_DRIVER_VERSIONPRINTER_H

#include "llvm/Support/raw_ostream.h"

namespace juice {
    namespace driver {
        class VersionPrinter {
        public:
            VersionPrinter() = delete;

            static void print(llvm::raw_ostream & os);
        };
    }
}

#endif //JUICE_DRIVER_VERSIONPRINTER_H
