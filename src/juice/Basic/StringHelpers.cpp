// src/juice/Basic/StringHelpers.cpp - Helper functions for working with strings
//
// This source file is part of the juice open source project
//
// Copyright (c) 2019 juice project authors
// Licensed under MIT License
//
// See https://github.com/juice-lang/juice/blob/master/LICENSE for license information
// See https://github.com/juice-lang/juice/blob/master/CONTRIBUTORS.txt for the list of juice project authors


#include "juice/Basic/StringHelpers.h"

namespace juice {
    namespace basic {
        std::string resize(std::string str, int size) {
            str.resize(size, ' ');
            return str;
        }

        char toLower(char c) {
            if (c >= 'A' && c <= 'Z') return c - 'A' + 'a';
            return c;
        }

        char toUpper(char c) {
            if (c >= 'a' && c <= 'z') return c - 'a' + 'A';
            return c;
        }

        bool isAlpha(char c) {
            return (c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z');
        }

        bool isDigit(char c) {
            return c >= '0' && c <= '9';
        }

        bool isAlNum(char c) {
            return isAlpha(c) || isDigit(c);
        }

        bool isIdentifierHead(char c) {
            return isAlpha(c) || c == '_';
        }

        bool isIdentifierChar(char c) {
            return isAlNum(c) || c == '_';
        }
    }
}
