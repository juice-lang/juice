// include/juice/Parser/Lexer.h - Lexer class, tokenizes a source file
//
// This source file is part of the juice open source project
//
// Copyright (c) 2019 - 2020 juice project authors
// Licensed under MIT License
//
// See https://github.com/juice-lang/juice/blob/master/LICENSE for license information
// See https://github.com/juice-lang/juice/blob/master/CONTRIBUTORS.txt for the list of juice project authors


#ifndef JUICE_PARSER_LEXER_H
#define JUICE_PARSER_LEXER_H

#include <cstddef>
#include <memory>

#include "LexerToken.h"
#include "juice/Basic/SourceBuffer.h"
#include "juice/Diagnostics/Diagnostics.h"

namespace juice {
    namespace parser {
        class Lexer {
            std::shared_ptr<basic::SourceBuffer> _sourceBuffer;

            const char * _start;
            const char * _current;

            char peek();
            char peekNext();
            bool isAtEnd();

            char advance();
            void advanceBy(size_t amount);
            bool match(char expected);

            std::unique_ptr<LexerToken> makeToken(LexerToken::Type type);
            std::unique_ptr<LexerToken> errorToken(diag::DiagnosticID id, bool atEnd = false);
            std::unique_ptr<LexerToken> errorToken(diag::DiagnosticID id, const char * position);

            void skipLineComment();
            bool skipBlockComment();

            LexerToken::Type checkKeyword(int startCount, int length, const char * rest, LexerToken::Type type);
            LexerToken::Type identifierType();

            std::unique_ptr<LexerToken> stringLiteral();
            std::unique_ptr<LexerToken> identifier();
            std::unique_ptr<LexerToken> numberLiteral();

        public:
            Lexer() = delete;
            Lexer(const Lexer &) = delete;
            Lexer & operator=(const Lexer &) = delete;

            explicit Lexer(std::shared_ptr<basic::SourceBuffer> sourceBuffer);

            std::unique_ptr<LexerToken> nextToken();
        };
    }
}

#endif //JUICE_PARSER_LEXER_H
