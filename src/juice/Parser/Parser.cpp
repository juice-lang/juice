// src/juice/Parser/Parser.cpp - Parser class, parses source file into an AST
//
// This source file is part of the juice open source project
//
// Copyright (c) 2019 juice project authors
// Licensed under MIT License
//
// See https://github.com/juice-lang/juice/blob/master/LICENSE for license information
// See https://github.com/juice-lang/juice/blob/master/CONTRIBUTORS.txt for the list of juice project authors


#include "juice/Parser/Parser.h"

namespace juice {
    namespace parser {
        Parser::Parser(const std::shared_ptr<diag::DiagnosticEngine> & diagnostics): _diagnostics(diagnostics) {
            _lexer = std::make_unique<Lexer>(diagnostics->getBuffer());
        }
    }
}
