// include/juice/Sema/TypeHint.h - type hint classes
//
// This source file is part of the juice open source project
//
// Copyright (c) 2019 - 2020 juice project authors
// Licensed under MIT License
//
// See https://github.com/juice-lang/juice/blob/master/LICENSE for license information
// See https://github.com/juice-lang/juice/blob/master/CONTRIBUTORS.txt for the list of juice project authors


#ifndef JUICE_SEMA_TYPEHINT_H
#define JUICE_SEMA_TYPEHINT_H

#include <type_traits>

#include "Type.h"
#include "juice/Basic/SFINAE.h"
#include "llvm/ADT/StringRef.h"

namespace juice {
    namespace sema {
        class TypeHint {
        public:
            enum class Kind {
                none,
                unknown,
                expected
            };

            enum class Flags: uint8_t {
                lValue = 1 << 0,
                rValue = 1 << 1
            };

        private:
            Kind _kind;
            uint8_t _flags = 0;

        protected:
            explicit TypeHint(Kind kind): _kind(kind) {}

            template <typename... T, std::enable_if_t<basic::all_same<Flags, T...>::value> * = nullptr>
            TypeHint(Kind kind, Flags flag, T... flags): TypeHint(kind, flags...) {
                _flags |= (uint8_t)flag;
            }

        public:
            TypeHint() = delete;

            Kind getKind() const { return _kind; }

            bool requiresRValue() const { return _flags & (uint8_t)Flags::rValue; }
            bool requiresLValue() const { return _flags & (uint8_t)Flags::lValue; }
        };

        class NoneTypeHint: public TypeHint {
        public:
            template <typename... T, std::enable_if_t<basic::all_same<Flags, T...>::value> * = nullptr>
            explicit NoneTypeHint(T... flags): TypeHint(Kind::none, flags...) {}


            static bool classof(const TypeHint * hint) {
                return hint->getKind() == Kind::none;
            }
        };

        class UnknownTypeHint: public TypeHint {
        public:
            template <typename... T, std::enable_if_t<basic::all_same<Flags, T...>::value> * = nullptr>
            explicit UnknownTypeHint(T... flags): TypeHint(Kind::unknown, flags...) {}


            static bool classof(const TypeHint * hint) {
                return hint->getKind() == Kind::unknown;
            }
        };

        class ExpectedTypeHint: public TypeHint {
            Type _type;

        public:
            template <typename... T, std::enable_if_t<basic::all_same<Flags, T...>::value> * = nullptr>
            explicit ExpectedTypeHint(Type type, T... flags): TypeHint(Kind::expected, flags...), _type(type) {}

            Type getType() const { return _type; }


            static bool classof(const TypeHint * hint) {
                return hint->getKind() == Kind::expected;
            }
        };
    }
}

#endif //JUICE_SEMA_TYPEHINT_H
