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

#include "llvm/Support/ErrorHandling.h"

namespace juice {
    namespace sema {
        class BuiltinType: public TypeBase {
        protected:
            explicit BuiltinType(Kind kind): TypeBase(kind) {}

        public:
            BuiltinType() = delete;
            BuiltinType(const BuiltinType &) = delete;
            void operator=(const BuiltinType &) = delete;

            virtual bool equals(const TypeBase * other) const = 0;


            static bool classof(const TypeBase * type) {
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

            explicit BuiltinIntegerType(Width width): BuiltinType(Kind::builtinInteger), _width(width) {}

        public:
            BuiltinIntegerType() = delete;
            BuiltinIntegerType(const BuiltinIntegerType &) = delete;
            void operator=(const BuiltinIntegerType &) = delete;


            static const BuiltinIntegerType * getBool() {
                static const BuiltinIntegerType _bool(Width::_1);

                return &_bool;
            }

            static const BuiltinIntegerType * getInt8() {
                static const BuiltinIntegerType int8(Width::_8);

                return &int8;
            }

            static const BuiltinIntegerType * getInt16() {
                static const BuiltinIntegerType int16(Width::_16);

                return &int16;
            }

            static const BuiltinIntegerType * getInt32() {
                static const BuiltinIntegerType int32(Width::_32);

                return &int32;
            }

            static const BuiltinIntegerType * getInt64() {
                static const BuiltinIntegerType int64(Width::_64);

                return &int64;
            }

            static const BuiltinIntegerType * getInt128() {
                static const BuiltinIntegerType int128(Width::_128);

                return &int128;
            }

            static const BuiltinIntegerType * getNativeWidth() {
                int wordBits = sizeof(void *) * 8;

                switch (wordBits) {
                    case 8: return getInt8();
                    case 16: return getInt16();
                    case 32: return getInt32();
                    case 64: return getInt64();
                    default:
                        llvm_unreachable("non-standard integer size is not supported");
                }
            }


            llvm::Type * toLLVM(llvm::LLVMContext & context) const override;


            bool isBool() const { return _width == Width::_1; }

            Width getWidth() const { return _width; }
            unsigned int getBitWidth() const { return (unsigned int)_width; }

            int64_t getMinimumValue() const;
            int64_t getMaximumValue() const;

            bool equals(const TypeBase * other) const override;


            static bool classof(const TypeBase * type) {
                return type->getKind() == Kind::builtinInteger;
            }
        };

        class BuiltinFloatingPointType: public BuiltinType {
        public:
            enum class FPKind {
                ieee16,
                ieee32,
                ieee64,
                ieee128
            };

        private:
            FPKind _fpKind;

            explicit BuiltinFloatingPointType(FPKind fpKind):
                BuiltinType(Kind::builtinFloatingPoint), _fpKind(fpKind) {}

        public:
            BuiltinFloatingPointType() = delete;
            BuiltinFloatingPointType(const BuiltinFloatingPointType &) = delete;
            void operator=(const BuiltinFloatingPointType &) = delete;


            static const BuiltinFloatingPointType * getFloat() {
                static const BuiltinFloatingPointType _float(FPKind::ieee32);

                return &_float;
            }

            static const BuiltinFloatingPointType * getDouble() {
                static const BuiltinFloatingPointType _double(FPKind::ieee64);

                return &_double;
            }


            llvm::Type * toLLVM(llvm::LLVMContext & context) const override;


            bool isFloat() const { return _fpKind == FPKind::ieee32; }
            bool isDouble() const { return _fpKind == FPKind::ieee64; }

            FPKind getFPKind() const { return _fpKind; }

            unsigned int getBitWidth() const {
                switch (_fpKind) {
                    case FPKind::ieee16:  return 16;
                    case FPKind::ieee32:  return 32;
                    case FPKind::ieee64:  return 64;
                    case FPKind::ieee128: return 128;
                }
            }

            bool equals(const TypeBase * other) const override;


            static bool classof(const TypeBase * type) {
                return type->getKind() == Kind::builtinFloatingPoint;
            }
        };
    }
}

#endif //JUICE_SEMA_BUILTINTYPE_H
