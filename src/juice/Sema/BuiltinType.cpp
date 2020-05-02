// src/juice/Sema/BuiltinType.cpp - juice builtin type classes
//
// This source file is part of the juice open source project
//
// Copyright (c) 2019 - 2020 juice project authors
// Licensed under MIT License
//
// See https://github.com/juice-lang/juice/blob/master/LICENSE for license information
// See https://github.com/juice-lang/juice/blob/master/CONTRIBUTORS.txt for the list of juice project authors


#include "juice/Sema/BuiltinType.h"

#include "llvm/Support/Casting.h"

namespace juice {
    namespace sema {
        bool BuiltinIntegerType::equals(const TypeBase * other) const {
            if (auto otherInteger = llvm::dyn_cast<BuiltinIntegerType>(other)) {
                return this->_width == otherInteger->_width;
            }

            return false;
        }

        bool BuiltinFloatingPointType::equals(const TypeBase * other) const {
            if (auto otherFloat = llvm::dyn_cast<BuiltinFloatingPointType>(other)) {
                return this->_fpKind == otherFloat->_fpKind;
            }

            return false;
        }
    }
}

