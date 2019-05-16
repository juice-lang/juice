// src/juice/Parser/Lexer.cpp - Lexer class, tokenizes a source file
//
// This source file is part of the juice open source project
//
// Copyright (c) 2019 juice project authors
// Licensed under MIT License
//
// See https://github.com/juice-lang/juice/blob/master/LICENSE for license information
// See https://github.com/juice-lang/juice/blob/master/CONTRIBUTORS.txt for the list of juice project authors


#include "juice/Parser/Lexer.h"

#include "juice/Basic/StringRef.h"

namespace juice {
    namespace parser {
        char Lexer::peek() {
            return *_current;
        }

        char Lexer::peekNext() {
            if (isAtEnd()) return 0;
            return _current[1];
        }

        bool Lexer::isAtEnd() {
            return _current == _sourceBuffer->getEnd();
        }

        char Lexer::advance() {
            char c = peek();
            _current++;
            return c;
        }

        void Lexer::advanceBy(size_t amount) {
            _current += amount;
        }

        bool Lexer::match(char expected) {
            if (isAtEnd()) return false;
            if (peek() != expected) return false;

            advance();
            return true;
        }

        std::unique_ptr<LexerToken> Lexer::makeToken(LexerToken::Type type) {
            basic::StringRef string(_start, _current - _start);
            return std::make_unique<LexerToken>(type, string);
        }

        std::unique_ptr<LexerToken> Lexer::errorToken(diag::DiagnosticID id, bool atEnd) {
            basic::StringRef string(_start, _current - _start);
            return std::make_unique<ErrorToken>(string, id, atEnd ? _current : _start);
        }

        std::unique_ptr<LexerToken> Lexer::errorToken(diag::DiagnosticID id, const char * position) {
            basic::StringRef string(_start, _current - _start);
            return std::make_unique<ErrorToken>(string, id, position);
        }

        std::unique_ptr<LexerToken> Lexer::nextToken() {
            if (isAtEnd()) return makeToken(LexerToken::Type::eof);

            while(!isAtEnd()) {
                _start = _current;

                char c = advance();

                switch (c) {
                    case '\n': return makeToken(LexerToken::Type::delimiterNewline);
                    case ' ':
                    case '\r':
                    case '\t': {
                        while (peek() == ' ' || peek() == '\r' || peek() == '\t') {
                            advance();
                        }
                        break;
                    }
                    case '(': return makeToken(LexerToken::Type::delimiterLeftParen);
                    case ')': return makeToken(LexerToken::Type::delimiterRightParen);
                    case '{': return makeToken(LexerToken::Type::delimiterLeftBrace);
                    case '}': return makeToken(LexerToken::Type::delimiterRightBrace);
                    case '[': return makeToken(LexerToken::Type::delimiterLeftBracket);
                    case ']': return makeToken(LexerToken::Type::delimiterRightBracket);
                    case ':': return makeToken(LexerToken::Type::delimiterColon);
                    case ';': return makeToken(LexerToken::Type::delimiterSemicolon);
                    case ',': return makeToken(LexerToken::Type::delimiterComma);
                    case '.': return makeToken(LexerToken::Type::operatorDot);
                    case '!': return makeToken(match('=') ? LexerToken::Type::operatorBangEqual
                                                          : LexerToken::Type::operatorBang);
                    case '=': return makeToken(match('=') ? LexerToken::Type::operatorEqualEqual
                                                          : LexerToken::Type::operatorEqual);
                    case '<': return makeToken(match('=') ? LexerToken::Type::operatorLowerEqual
                                                          : LexerToken::Type::operatorLower);
                    case '>': return makeToken(match('=') ? LexerToken::Type::operatorGreaterEqual
                                                          : LexerToken::Type::operatorGreater);
                    case '&': return makeToken(match('&') ? LexerToken::Type::operatorAndAnd
                                                          : LexerToken::Type::operatorAnd);
                    case '|': return makeToken(match('|') ? LexerToken::Type::operatorPipePipe
                                                          : LexerToken::Type::operatorPipe);
                    case '+': return makeToken(match('=') ? LexerToken::Type::operatorPlusEqual
                                                          : LexerToken::Type::operatorPlus);
                    case '-': return makeToken(match('=') ? LexerToken::Type::operatorMinusEqual
                                                          : LexerToken::Type::operatorMinus);
                    case '*': return makeToken(match('=') ? LexerToken::Type::operatorAsteriskEqual
                                                          : LexerToken::Type::operatorAsterisk);
                    case '%': return makeToken(match('=') ? LexerToken::Type::operatorPercentEqual
                                                          : LexerToken::Type::operatorPercent);
                    default: {
                        return errorToken(diag::DiagnosticID::invalid_character);
                    }
                }
            }

            return makeToken(LexerToken::Type::eof);
        }
    }
}
