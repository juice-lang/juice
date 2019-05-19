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

#include <utility>

namespace juice {
    namespace parser {
        bool Parser::isAtEnd() {
            return _currentToken != nullptr && _currentToken->type == LexerToken::Type::eof;
        }

        bool Parser::check(LexerToken::Type type) {
            if (isAtEnd()) return false;
            return _currentToken->type == type;
        }

        void Parser::advance() {
            _previousToken = std::move(_currentToken);
            if (!isAtEnd()) _currentToken = _lexer->nextToken();
            if (check(LexerToken::Type::error)) throw LexerError();
        }

        bool Parser::match(LexerToken::Type type) {
            if (check(type)) {
                advance();
                return true;
            }
            return false;
        }

        void Parser::consume(LexerToken::Type type, diag::DiagnosticID errorID) {
            if (check(type)) advance();
            else throw Error(errorID);
        }

        Parser::Parser(std::shared_ptr<diag::DiagnosticEngine> diagnostics):
                _diagnostics(std::move(diagnostics)), _previousToken(nullptr), _currentToken(nullptr) {
            _lexer = std::make_unique<Lexer>(_diagnostics->getBuffer());
        }
    }
}
