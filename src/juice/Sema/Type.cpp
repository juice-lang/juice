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
        bool Type::isBuiltinBool() const {
            return _pointer->getKind() == TypeBase::Kind::builtinInteger
                && llvm::cast<BuiltinIntegerType>(_pointer)->isBool();
        }

        bool Type::isBuiltinDouble() const {
            return _pointer->getKind() == TypeBase::Kind::builtinFloatingPoint
                && llvm::cast<BuiltinFloatingPointType>(_pointer)->isDouble();
        }

        bool Type::operator==(Type other) const {
            switch (_pointer->getKind()) {
                case TypeBase::Kind::_void: return llvm::isa<VoidType>(other._pointer);
                case TypeBase::Kind::nothing: return llvm::isa<NothingType>(other._pointer);
                case TypeBase::Kind::builtin:
                case TypeBase::Kind::builtinInteger:
                case TypeBase::Kind::builtinFloatingPoint:
                case TypeBase::Kind::builtin_last:
                    return llvm::cast<BuiltinType>(_pointer)->equals(other._pointer);
            }
        }

        llvm::Type * VoidType::toLLVM(llvm::LLVMContext & context) const {
            return llvm::Type::getVoidTy(context);
        }

        llvm::Type * NothingType::toLLVM(llvm::LLVMContext & context) const {
            return nullptr;
        }


        llvm::raw_ostream & operator<<(llvm::raw_ostream & os, Type type) {
            switch (type.getPointer()->getKind()) {
                case TypeBase::Kind::_void:
                    os << "()";
                    break;
                case TypeBase::Kind::nothing:
                    os << "--";
                    break;
                case TypeBase::Kind::builtinInteger: {
                    auto integerType = llvm::cast<BuiltinIntegerType>(type.getPointer());
                    os << "Builtin::Int" << integerType->getBitWidth();
                    break;
                }
                case TypeBase::Kind::builtinFloatingPoint: {
                    auto floatingPointType = llvm::cast<BuiltinFloatingPointType>(type.getPointer());
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
