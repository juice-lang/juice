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

#include "juice/Basic/StringHelpers.h"
#include "juice/Basic/StringRef.h"
#include "juice/Parser/FSM.h"

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

        void Lexer::skipLineComment() {
            while (peek() != '\n' && !isAtEnd()) advance();
        }

        bool Lexer::skipBlockComment() {
            int nesting = 1;
            while (nesting > 0) {
                if (isAtEnd()) return false;

                if (peek() == '/' && peekNext() == '*') {
                    advanceBy(2);
                    nesting++;
                    continue;
                }

                if (peek() == '*' && peekNext() == '/') {
                    advanceBy(2);
                    nesting--;
                    continue;
                }

                advance();
            }

            return true;
        }

        LexerToken::Type Lexer::checkKeyword(int startCount, int length, const char * rest,
                                             juice::parser::LexerToken::Type type) {
            if (_current - _start == startCount + length && memcmp(_start + startCount, rest, length) == 0) {
                return type;
            }

            return LexerToken::Type::identifier;
        }

        LexerToken::Type Lexer::identifierType() {
            switch (_start[0]) {
                case 'a': return checkKeyword(1, 1, "s", LexerToken::Type::keywordAs);
                case 'b': {
                    if (_current - _start > 1) {
                        switch (_start[1]) {
                            case 'i': return checkKeyword(2, 4, "nary", LexerToken::Type::keywordBinary);
                            case 'r': return checkKeyword(2, 3, "eak", LexerToken::Type::keywordBreak);
                        }
                    }
                    break;
                }
                case 'c': {
                    if (_current - _start > 1) {
                        switch (_start[1]) {
                            case 'a': return checkKeyword(2, 2, "se", LexerToken::Type::keywordCase);
                            case 'l': return checkKeyword(2, 3, "ass", LexerToken::Type::keywordClass);
                            case 'o': {
                                if (_current - _start > 2) {
                                    switch (_start[2]) {
                                        case 'm': return checkKeyword(3, 5, "pound", LexerToken::Type::keywordCompound);
                                        case 'n': return checkKeyword(3, 5, "tinue", LexerToken::Type::keywordContinue);
                                    }
                                }
                            }
                        }
                    }
                    break;
                }
                case 'd': return checkKeyword(1, 1, "o", LexerToken::Type::keywordDo);
                case 'e': {
                    if (_current - _start > 2 && _start[1] == 'l') {
                        switch (_start[2]) {
                            case 'i': return checkKeyword(3, 1, "f", LexerToken::Type::keywordElif);
                            case 's': return checkKeyword(3, 1, "e", LexerToken::Type::keywordElse);
                        }
                    }
                    break;
                }
                case 'f': {
                    if (_current - _start > 1) {
                        switch (_start[1]) {
                            case 'a': {
                                if (_current - _start > 2) {
                                    switch (_start[2]) {
                                        case 'i': return checkKeyword(3, 5, "lable", LexerToken::Type::keywordFailable);
                                        case 'l': return checkKeyword(3, 2, "se", LexerToken::Type::keywordFalse);
                                    }
                                }
                                break;
                            }
                            case 'o': return checkKeyword(2, 1, "r", LexerToken::Type::keywordFor);
                            case 'u': return checkKeyword(2, 2, "nc", LexerToken::Type::keywordFunc);
                        }
                    }
                    break;
                }
                case 'i': {
                    if (_current - _start > 1) {
                        switch (_start[1]) {
                            case 'f': return checkKeyword(2, 0, "", LexerToken::Type::keywordIf);
                            case 'n':
                                if (_current - _start > 2) {
                                    return checkKeyword(2, 2, "it", LexerToken::Type::keywordInit);
                                } else {
                                    return LexerToken::Type::keywordIn;
                                }
                            case 's': return checkKeyword(2, 0, "", LexerToken::Type::keywordIs);
                        }
                    }
                    break;
                }
                case 'l': return checkKeyword(1, 2, "et", LexerToken::Type::keywordLet);
                case 'n': return checkKeyword(1, 2, "il", LexerToken::Type::keywordNil);
                case 'o': return checkKeyword(1, 7, "verride", LexerToken::Type::keywordOverride);
                case 'p': {
                    if (_current - _start > 3 && _start[1] == 'r' && _start[2] == 'i') {
                        switch (_start[3]) {
                            case 'n': return checkKeyword(4, 1, "t", LexerToken::Type::keywordPrint);
                            case 'v': return checkKeyword(4, 3, "ate", LexerToken::Type::keywordPrivate);
                        }
                    }
                    break;
                }
                case 'r': return checkKeyword(1, 5, "eturn", LexerToken::Type::keywordReturn);
                case 's': {
                    if (_current - _start > 1) {
                        switch (_start[1]) {
                            case 'e': return checkKeyword(2, 2, "lf", LexerToken::Type::keywordSelf);
                            case 'u': return checkKeyword(2, 3, "per", LexerToken::Type::keywordSuper);
                            case 'w': return checkKeyword(2, 4, "itch", LexerToken::Type::keywordSwitch);
                        }
                    }
                    break;
                }
                case 't': return checkKeyword(1, 3, "rue", LexerToken::Type::keywordTrue);
                case 'u': return checkKeyword(1, 4, "nary", LexerToken::Type::keywordUnary);
                case 'v': return checkKeyword(1, 2, "ar", LexerToken::Type::keywordVar);
                case 'w': return checkKeyword(1, 4, "hile", LexerToken::Type::keywordWhile);
            }

            return LexerToken::Type::identifier;
        }

        std::unique_ptr<LexerToken> Lexer::stringLiteral() {
            FSM::Return result = StringFSM::run(_start);

            advanceBy(result.length - 1);

            if (result.error == nullptr) return makeToken(LexerToken::Type::stringLiteral);
            if (result.state == StringFSM::invalidEscapeEnd)
                return errorToken(diag::DiagnosticID::invalid_escape, result.error);

            return errorToken(diag::DiagnosticID::unterminated_string, result.error);
        }

        std::unique_ptr<LexerToken> Lexer::identifier() {
            while (basic::isIdentifierChar(peek())) advance();

            return makeToken(identifierType());
        }

        std::unique_ptr<LexerToken> Lexer::numberLiteral() {
            FSM::Return result = NumberFSM::run(_start);

            advanceBy(result.length - 1);

            if (result.error == nullptr)
                return makeToken(result.state == NumberFSM::integer ? LexerToken::Type::integerLiteral
                                                                    : LexerToken::Type::decimalLiteral);

            if (result.state == NumberFSM::begin) assert(false);
            if (result.state == NumberFSM::beginDecimal)
                return errorToken(diag::DiagnosticID::expected_digit_decimal_sign, result.error);

            return errorToken(diag::DiagnosticID::expected_digit_exponent, result.error);
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
                    case '/': {
                        if (match('/')) {
                            skipLineComment();
                            break;
                        }

                        if (match('*')) {
                            if (!skipBlockComment()) return errorToken(diag::DiagnosticID::unterminated_comment);
                            break;
                        }

                        return makeToken(match('=') ? LexerToken::Type::operatorSlashEqual
                                                    : LexerToken::Type::operatorSlash);
                    }
                    case '"': return stringLiteral();
                    default: {
                        if (basic::isIdentifierHead(c)) return identifier();
                        if (basic::isDigit(c)) return numberLiteral();
                        return errorToken(diag::DiagnosticID::invalid_character);
                    }
                }
            }

            return makeToken(LexerToken::Type::eof);
        }
    }
}
