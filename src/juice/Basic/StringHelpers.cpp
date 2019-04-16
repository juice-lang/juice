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
    }
}
