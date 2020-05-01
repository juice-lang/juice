// include/juice/Sema/BuiltinType.h - juice builtin type classes
//
// This source file is part of the juice open source project
//
// Copyright (c) 2019 - 2020 juice project authors
// Licensed under MIT License
//
// See https://github.com/juice-lang/juice/blob/master/LICENSE for license information
// See https://github.com/juice-lang/juice/blob/master/CONTRIBUTORS.txt for the list of juice project authors


#ifndef JUICE_SEMA_BUILTINTYPE_H
#define JUICE_SEMA_BUILTINTYPE_H

#include "Type.h"

namespace juice {
    namespace sema {
        class BuiltinType: public Type {
        public:
            template <typename... T, std::enable_if_t<basic::all_same<Flags, T...>::value> * = nullptr>
            explicit BuiltinType(Kind kind, T... flags): Type(kind, flags...) {}


            static bool classof(const Type * type) {
                return type->getKind() >= Kind::builtin
                    && type->getKind() <= Kind::builtin_last;
            }
        };

        class BuiltinIntegerType: public BuiltinType {
        public:
            enum class Width: unsigned int {
                _1 =   1,
                _8 =   8,
                _16 =  16,
                _32 =  32,
                _64 =  64,
                _128 = 128
            };

        private:
            Width _width;

        public:
            template <typename... T, std::enable_if_t<basic::all_same<Flags, T...>::value> * = nullptr>
            explicit BuiltinIntegerType(Width width, T... flags):
                BuiltinType(Kind::builtinInteger, flags...), _width(width) {}

            Width getWidth() const { return _width; }
            unsigned int getBitWidth() const { return (unsigned int)_width; }


            static const BuiltinIntegerType * getBool() {
                static const BuiltinIntegerType _bool(Width::_1);

                return &_bool;
            }


            static bool classof(const Type * type) {
                return type->getKind() == Kind::builtinInteger;
            }
        };

        class BuiltinFloatingPointType: public BuiltinType {
        public:
            enum class FPKind {
                ieee16,
                ieee32,
                ieee64,
                ieee80,
                ieee128
            };

        private:
            FPKind _fpKind;

        public:
            template <typename... T, std::enable_if_t<basic::all_same<Flags, T...>::value> * = nullptr>
            explicit BuiltinFloatingPointType(FPKind fpKind, T... flags):
                BuiltinType(Kind::builtinFloatingPoint, flags...), _fpKind(fpKind) {}

            FPKind getFPKind() const { return _fpKind; }

            unsigned int getBitWidth() const {
                switch (_fpKind) {
                    case FPKind::ieee16:  return 16;
                    case FPKind::ieee32:  return 32;
                    case FPKind::ieee64:  return 64;
                    case FPKind::ieee80:  return 80;
                    case FPKind::ieee128: return 128;
                }
            }


            static const BuiltinFloatingPointType * getFloat() {
                static const BuiltinFloatingPointType _float(FPKind::ieee32);

                return &_float;
            }

            static const BuiltinFloatingPointType * getDouble() {
                static const BuiltinFloatingPointType _double(FPKind::ieee64);

                return &_double;
            }


            static bool classof(const Type * type) {
                return type->getKind() == Kind::builtinFloatingPoint;
            }
        };
    }
}

#endif //JUICE_SEMA_BUILTINTYPE_H
