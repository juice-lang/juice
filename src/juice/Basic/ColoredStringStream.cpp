// src/juice/Basic/ColoredStringStream.cpp - A string stream that is conditionally colored
//
// This source file is part of the juice open source project
//
// Copyright (c) 2019 - 2022 juice project authors
// Licensed under MIT License
//
// See https://github.com/juice-lang/juice/blob/master/LICENSE for license information
// See https://github.com/juice-lang/juice/blob/master/CONTRIBUTORS.txt for the list of juice project authors


#include "juice/Basic/ColoredStringStream.h"

namespace juice {
    namespace basic {
        ColoredStringStream::ColoredStringStream(std::string & string, bool isColored):
            llvm::raw_string_ostream(string), _isColored(isColored) {}
    }
}
