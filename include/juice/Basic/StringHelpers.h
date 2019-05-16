// include/juice/Basic/StringHelpers.h - Helper functions for working with strings
//
// This source file is part of the juice open source project
//
// Copyright (c) 2019 juice project authors
// Licensed under MIT License
//
// See https://github.com/juice-lang/juice/blob/master/LICENSE for license information
// See https://github.com/juice-lang/juice/blob/master/CONTRIBUTORS.txt for the list of juice project authors


#ifndef JUICE_STRINGHELPERS_H
#define JUICE_STRINGHELPERS_H

#include <string>

namespace juice {
    namespace basic {
        std::string resize(std::string str, int size);

        char toLower(char c);
        char toUpper(char c);

        bool isAlpha(char c);
        bool isDigit(char c);
        bool isAlNum(char c);

        bool isIdentifierHead(char c);
        bool isIdentifierChar(char c);
    }
}

#endif //JUICE_STRINGHELPERS_H
