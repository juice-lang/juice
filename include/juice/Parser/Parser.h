// include/juice/Parser/Parser.h - Parser class, parses source file into an AST
//
// This source file is part of the juice open source project
//
// Copyright (c) 2019 juice project authors
// Licensed under MIT License
//
// See https://github.com/juice-lang/juice/blob/master/LICENSE for license information
// See https://github.com/juice-lang/juice/blob/master/CONTRIBUTORS.txt for the list of juice project authors


#ifndef JUICE_PARSER_H
#define JUICE_PARSER_H

#include <memory>

#include "Lexer.h"
#include "LexerToken.h"
#include "juice/Diagnostics/Diagnostics.h"

namespace juice {
    namespace parser {
        class Parser {
            std::shared_ptr<diag::DiagnosticEngine> _diagnostics;
            std::unique_ptr<Lexer> _lexer;

            std::unique_ptr<LexerToken> currentToken() const { return _lexer->nextToken(); }

        public:
            Parser() = delete;
            Parser(const Parser &) = delete;
            Parser & operator=(const Parser &) = delete;

            explicit Parser(std::shared_ptr<diag::DiagnosticEngine> diagnostics);
        };
    }
}

#endif //JUICE_PARSER_H
