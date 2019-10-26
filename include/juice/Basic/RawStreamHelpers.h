// include/juice/Basic/RawStreamHelpers.h - Helper functions for working with llvm raw_ostreams
//
// This source file is part of the juice open source project
//
// Copyright (c) 2019 juice project authors
// Licensed under MIT License
//
// See https://github.com/juice-lang/juice/blob/master/LICENSE for license information
// See https://github.com/juice-lang/juice/blob/master/CONTRIBUTORS.txt for the list of juice project authors


#ifndef JUICE_RAWSTREAMHELPERS_H
#define JUICE_RAWSTREAMHELPERS_H

#include <string>

#include "llvm/Support/raw_ostream.h"

namespace juice {
    namespace basic {
        class Color {
        public:
            enum Value : uint8_t {
                black = 0,
                red,
                green,
                yellow,
                blue,
                magenta,
                cyan,
                white,
                bold,
                reset
            };

            Color() = default;
            /* implicit */ constexpr Color(Value value) : _value(value) {}

            /* implicit */ constexpr operator Value() const { return _value; }

            explicit operator bool() = delete;
            constexpr bool operator==(Color other) const { return _value == other._value; }
            constexpr bool operator==(Value other) const { return _value == other; }
            constexpr bool operator!=(Color other) const { return _value != other._value; }
            constexpr bool operator!=(Value other) const { return _value != other; }

        private:
            Value _value;
        };

        llvm::raw_ostream & operator<<(llvm::raw_ostream & os, Color::Value color);
    }
}

#endif //JUICE_RAWSTREAMHELPERS_H
