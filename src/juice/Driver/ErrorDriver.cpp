// src/juice/Driver/ErrorDriver.cpp - Driver subclass for argument parsing errors
//
// This source file is part of the juice open source project
//
// Copyright (c) 2019 - 2020 juice project authors
// Licensed under MIT License
//
// See https://github.com/juice-lang/juice/blob/master/LICENSE for license information
// See https://github.com/juice-lang/juice/blob/master/CONTRIBUTORS.txt for the list of juice project authors


#include "juice/Driver/ErrorDriver.h"

#include "juice/Diagnostics/Diagnostics.h"
#include "llvm/ADT/StringRef.h"

namespace juice {
    namespace driver {
        int ErrorDriver::execute() {
            llvm::StringRef string(_message);
            diag::DiagnosticEngine::diagnose(diag::DiagnosticID::error_parsing_args, string);
            return 1;
        }
    }
}
