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

#include "llvm/IR/DerivedTypes.h"
#include "llvm/Support/Casting.h"

namespace juice {
    namespace sema {
        llvm::Type * BuiltinIntegerType::toLLVM(llvm::LLVMContext & context) const {
            switch (_width) {
                case Width::_1: return llvm::Type::getInt1Ty(context);
                case Width::_8: return llvm::Type::getInt8Ty(context);
                case Width::_16: return llvm::Type::getInt16Ty(context);
                case Width::_32: return llvm::Type::getInt32Ty(context);
                case Width::_64: return llvm::Type::getInt64Ty(context);
                case Width::_128: return llvm::Type::getInt128Ty(context);
            }
        }

        bool BuiltinIntegerType::equals(const TypeBase * other) const {
            if (auto otherInteger = llvm::dyn_cast<BuiltinIntegerType>(other)) {
                return this->_width == otherInteger->_width;
            }

            return false;
        }
        llvm::Type * BuiltinFloatingPointType::toLLVM(llvm::LLVMContext & context) const {
            switch (_fpKind) {
                case FPKind::ieee16: return llvm::Type::getHalfTy(context);
                case FPKind::ieee32: return llvm::Type::getFloatTy(context);
                case FPKind::ieee64: return llvm::Type::getDoubleTy(context);
                case FPKind::ieee128: return llvm::Type::getFP128Ty(context);
            }
        }

        bool BuiltinFloatingPointType::equals(const TypeBase * other) const {
            if (auto otherFloat = llvm::dyn_cast<BuiltinFloatingPointType>(other)) {
                return this->_fpKind == otherFloat->_fpKind;
            }

            return false;
        }
    }
}

