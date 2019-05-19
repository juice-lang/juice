// src/juice/Driver/CompilerDriver.cpp - Driver subclass for compiling a file
//
// This source file is part of the juice open source project
//
// Copyright (c) 2019 juice project authors
// Licensed under MIT License
//
// See https://github.com/juice-lang/juice/blob/master/LICENSE for license information
// See https://github.com/juice-lang/juice/blob/master/CONTRIBUTORS.txt for the list of juice project authors


#include "juice/Driver/CompilerDriver.h"

#include <memory>
#include <juice/Parser/Parser.h>

#include "juice/Basic/SourceBuffer.h"
#include "juice/Basic/StringRef.h"
#include "juice/Diagnostics/Diagnostics.h"
#include "juice/Parser/Lexer.h"
#include "juice/Parser/LexerToken.h"

namespace juice {
    namespace driver {
        int CompilerDriver::execute() {
            basic::StringRef filename(_filename);
            auto buffer = basic::SourceBuffer::getFile(filename);
            if (buffer == nullptr) {
                diag::DiagnosticEngine::diagnose(diag::DiagnosticID::file_not_found, filename);
                return 1;
            }

            auto diagnostics = std::make_shared<diag::DiagnosticEngine>(buffer);

            parser::Parser juiceParser(diagnostics);

            auto expression = juiceParser.parseProgram();

            return diagnostics->hadError() ? 1 : 0;
        }
    }
}
