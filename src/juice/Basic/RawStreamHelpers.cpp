// src/juice/Basic/RawStreamHelpers.cpp - Helper functions for working with llvm raw_ostreams
//
// This source file is part of the juice open source project
//
// Copyright (c) 2019 - 2020 juice project authors
// Licensed under MIT License
//
// See https://github.com/juice-lang/juice/blob/master/LICENSE for license information
// See https://github.com/juice-lang/juice/blob/master/CONTRIBUTORS.txt for the list of juice project authors

#include "juice/Basic/RawStreamHelpers.h"

namespace juice {
    namespace basic {
        const Color Color::rainbow[6] = {
            basic::Color::cyan,
            basic::Color::blue,
            basic::Color::magenta,
            basic::Color::red,
            basic::Color::yellow,
            basic::Color::green
        };

        llvm::raw_ostream & operator<<(llvm::raw_ostream & os, Color::Value color) {
            if (!os.has_colors()) return os;

            switch (color) {
                case Color::black: return os << "\033[30m";
                case Color::red: return os << "\033[31m";
                case Color::green: return os << "\033[32m";
                case Color::yellow: return os << "\033[33m";
                case Color::blue: return os << "\033[34m";
                case Color::magenta: return os << "\033[35m";
                case Color::cyan: return os << "\033[36m";
                case Color::white: return os << "\033[37m";
                case Color::bold: return os << "\033[1m";
                case Color::reset: return os << "\033[00m";
            }
        }
    }
}
