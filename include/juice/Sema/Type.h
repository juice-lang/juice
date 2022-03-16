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
#include "llvm/IR/LLVMContext.h"
#include "llvm/IR/Type.h"
#include "llvm/Support/raw_ostream.h"

namespace juice {
    namespace sema {
        class TypeBase {
        public:
            enum class Kind {
                _void,
                nothing,
                builtin,
                builtinInteger,
                builtinFloatingPoint,
                builtin_last
            };

        private:
            const Kind _kind;
            uint8_t _flags = 0;

        protected:
            explicit TypeBase(Kind kind): _kind(kind) {}

        public:
            TypeBase() = delete;
            TypeBase(const TypeBase &) = delete;
            void operator=(const TypeBase &) = delete;

            virtual ~TypeBase() = default;


            virtual llvm::Type * toLLVM(llvm::LLVMContext & context) const = 0;


            Kind getKind() const { return _kind; }
        };

        class VoidType: public TypeBase {
            VoidType(): TypeBase(Kind::_void) {}

        public:
            VoidType(const VoidType &) = delete;
            void operator=(const VoidType &) = delete;

            static const VoidType * get() {
                static const VoidType _void;

                return &_void;
            }


            llvm::Type * toLLVM(llvm::LLVMContext & context) const override;


            static bool classof(const TypeBase * type) {
                return type->getKind() == Kind::_void;
            }
        };

        class NothingType: public TypeBase {
            NothingType(): TypeBase(Kind::nothing) {}

        public:
            NothingType(const NothingType &) = delete;
            void operator=(const NothingType &) = delete;

            static const NothingType * get() {
                static const NothingType nothing;

                return &nothing;
            }


            llvm::Type * toLLVM(llvm::LLVMContext &context) const override;


            static bool classof(const TypeBase * type) {
                return type->getKind() == Kind::nothing;
            }
        };


        class Type {
        public:
            enum class Flags: uint8_t {
                lValue = 1 << 0
            };

        private:
            const TypeBase * _pointer = nullptr;
            uint8_t _flags = 0;

        public:
            Type() = default;

            // NOLINTNEXTLINE(google-explicit-constructor)
            /* implicit */ Type(const TypeBase * pointer): _pointer(pointer) {}

            // NOLINTNEXTLINE(google-explicit-constructor)
            /* implicit */ operator bool() const { return _pointer; }

            template <typename... T, std::enable_if_t<basic::all_same<Flags, T...>::value> * = nullptr>
            Type(const TypeBase * pointer, Flags flag, T... flags): Type(pointer, flags...) {
                addFlag(flag);
            }

            Type addFlag(Flags flag) {
                _flags |= (uint8_t)flag;
                return *this;
            }

            const TypeBase * getPointer() const { return _pointer; }
            const TypeBase * operator->() const { return _pointer; }

            bool isRValue() const { return !(_flags & (uint8_t)Flags::lValue); }
            bool isLValue() const { return _flags & (uint8_t)Flags::lValue; }


            bool isBuiltinInteger() const;
            bool isBuiltinBool() const;

            bool isBuiltinFloatingPoint() const;
            bool isBuiltinFloat() const;
            bool isBuiltinDouble() const;


            bool operator==(Type other) const;

            bool operator!=(Type other) const {
                return !(*this == other);
            }
        };

        llvm::raw_ostream & operator<<(llvm::raw_ostream & os, Type type);
    }
}

#endif //JUICE_SEMA_TYPE_H
