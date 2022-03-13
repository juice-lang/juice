// include/juice/Basic/ColoredStringStream.h - A string stream that is conditionally colored
//
// This source file is part of the juice open source project
//
// Copyright (c) 2019 - 2022 juice project authors
// Licensed under MIT License
//
// See https://github.com/juice-lang/juice/blob/master/LICENSE for license information
// See https://github.com/juice-lang/juice/blob/master/CONTRIBUTORS.txt for the list of juice project authors


#ifndef JUICE_BASIC_COLOREDSTRINGSTREAM_H
#define JUICE_BASIC_COLOREDSTRINGSTREAM_H

#include <string>

#include "llvm/Support/raw_ostream.h"

namespace juice {
    namespace basic {
        class ColoredStringStream: public llvm::raw_string_ostream {
            bool _isColored;

        public:
            ColoredStringStream(std::string & string, bool isColored);

            bool has_colors() const override { return _isColored; }
        };
    }
}

#endif //JUICE_BASIC_COLOREDSTRINGSTREAM_H
