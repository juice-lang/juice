// src/juice/Sema/Type.cpp - basic class for all types in juice
//
// This source file is part of the juice open source project
//
// Copyright (c) 2019 - 2020 juice project authors
// Licensed under MIT License
//
// See https://github.com/juice-lang/juice/blob/master/LICENSE for license information
// See https://github.com/juice-lang/juice/blob/master/CONTRIBUTORS.txt for the list of juice project authors


#include "juice/Sema/Type.h"

#include "juice/Sema/BuiltinType.h"
#include "llvm/Support/ErrorHandling.h"
#include "llvm/Support/Casting.h"

namespace juice {
    namespace sema {
        llvm::raw_ostream & operator<<(llvm::raw_ostream & os, const Type * type) {
            switch (type->getKind()) {
                case Type::Kind::_void:
                    os << "()";
                    break;
                case Type::Kind::nothing:
                    os << "--";
                    break;
                case Type::Kind::builtinInteger: {
                    auto integerType = llvm::cast<BuiltinIntegerType>(type);
                    os << "Builtin::Int" << integerType->getBitWidth();
                    break;
                }
                case Type::Kind::builtinFloatingPoint: {
                    auto floatingPointType = llvm::cast<BuiltinFloatingPointType>(type);
                    os << "Builtin::";

                    switch (floatingPointType->getFPKind()) {
                        case BuiltinFloatingPointType::FPKind::ieee16:
                            os << "Float16";
                            break;
                        case BuiltinFloatingPointType::FPKind::ieee32:
                            os << "Float";
                            break;
                        case BuiltinFloatingPointType::FPKind::ieee64:
                            os << "Double";
                            break;
                        case BuiltinFloatingPointType::FPKind::ieee80:
                            os << "Float80";
                            break;
                        case BuiltinFloatingPointType::FPKind::ieee128:
                            os << "Float128";
                            break;
                    }
                    break;
                }
                default:
                    llvm_unreachable("All possible Type kinds should be handled here");
            }

            return os;
        }
    }
}
