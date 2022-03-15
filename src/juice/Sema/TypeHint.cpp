// src/juice/Sema/TypeHint.cpp - type hint classes
//
// This source file is part of the juice open source project
//
// Copyright (c) 2019 - 2022 juice project authors
// Licensed under MIT License
//
// See https://github.com/juice-lang/juice/blob/master/LICENSE for license information
// See https://github.com/juice-lang/juice/blob/master/CONTRIBUTORS.txt for the list of juice project authors


#include "juice/Sema/TypeHint.h"

#include <algorithm>

namespace juice {
    namespace sema {
        bool ExpectedEitherTypeHint::matches(Type type) const {
            return std::any_of(_types.begin(), _types.end(), [=](Type allowedType) { return allowedType == type; });
        }
    }
}
