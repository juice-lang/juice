// include/juice/Sema/Type.h - basic class for all types in juice
//
// This source file is part of the juice open source project
//
// Copyright (c) 2019 - 2020 juice project authors
// Licensed under MIT License
//
// See https://github.com/juice-lang/juice/blob/master/LICENSE for license information
// See https://github.com/juice-lang/juice/blob/master/CONTRIBUTORS.txt for the list of juice project authors


#ifndef JUICE_SEMA_TYPE_H
#define JUICE_SEMA_TYPE_H

#include <type_traits>

#include "juice/Basic/SFINAE.h"
#include "llvm/Support/raw_ostream.h"

namespace juice {
    namespace sema {
        class Type {
        public:
            enum class Kind {
                _void,
                nothing,
                builtin,
                builtinInteger,
                builtinFloatingPoint,
                builtin_last
            };

            enum class Flags: uint8_t {
                lValue = 1 << 0
            };

        private:
            const Kind _kind;
            uint8_t _flags = 0;

        public:
            explicit Type(Kind kind): _kind(kind) {}

            template <typename... T, std::enable_if_t<basic::all_same<Flags, T...>::value> * = nullptr>
            Type(Kind kind, Flags flag, T... flags): Type(kind, flags...) {
                _flags |= (uint8_t)flag;
            }

            Kind getKind() const { return _kind; }

            bool isRValue() const { return !(_flags & (uint8_t)Flags::lValue); }
            bool isLValue() const { return _flags & (uint8_t)Flags::lValue; }
        };

        class VoidType: public Type {
        public:
            template <typename... T, std::enable_if_t<basic::all_same<Flags, T...>::value> * = nullptr>
            explicit VoidType(T... flags): Type(Kind::_void, flags...) {}


            static bool classof(const Type * type) {
                return type->getKind() == Kind::_void;
            }
        };

        class NothingType: public Type {
        public:
            template <typename... T, std::enable_if_t<basic::all_same<Flags, T...>::value> * = nullptr>
            explicit NothingType(T... flags): Type(Kind::nothing, flags...) {}


            static bool classof(const Type * type) {
                return type->getKind() == Kind::nothing;
            }
        };

        llvm::raw_ostream & operator<<(llvm::raw_ostream & os, const Type * type);
    }
}

#endif //JUICE_SEMA_TYPE_H