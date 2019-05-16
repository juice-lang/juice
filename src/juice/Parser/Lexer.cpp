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
        std::unique_ptr<LexerToken> Lexer::nextToken() {
            return std::make_unique<LexerToken>(LexerToken::Type::eof, basic::StringRef());
        }
    }
}
