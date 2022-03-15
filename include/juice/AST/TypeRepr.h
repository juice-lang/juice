// include/juice/AST/TypeRepr.h - Representation of a type as written in source
//
// This source file is part of the juice open source project
//
// Copyright (c) 2019 - 2022 juice project authors
// Licensed under MIT License
//
// See https://github.com/juice-lang/juice/blob/master/LICENSE for license information
// See https://github.com/juice-lang/juice/blob/master/CONTRIBUTORS.txt for the list of juice project authors


#ifndef JUICE_AST_TYPEREPR_H
#define JUICE_AST_TYPEREPR_H

#include <memory>

#include "juice/Parser/LexerToken.h"
#include "juice/Sema/Type.h"
#include "juice/Sema/TypeChecker.h"
#include "llvm/ADT/StringRef.h"
#include "llvm/Support/Error.h"
#include "llvm/Support/raw_ostream.h"

namespace juice {
    namespace ast {
        class TypeRepr {
        public:
            enum class Kind {
                identifier
            };

        private:
            const Kind _kind;

        protected:
            explicit TypeRepr(Kind kind): _kind(kind) {}

        public:
            TypeRepr() = delete;
            TypeRepr(const TypeRepr &) = delete;
            void operator=(const TypeRepr &) = delete;

            virtual ~TypeRepr() = default;


            virtual llvm::Expected<sema::Type> resolve(const sema::TypeChecker::State & state) const = 0;


            Kind getKind() const { return _kind; }
        };

        class IdentifierTypeRepr: public TypeRepr {
            std::unique_ptr<parser::LexerToken> _token;

        public:
            explicit IdentifierTypeRepr(std::unique_ptr<parser::LexerToken> token);

            IdentifierTypeRepr(const IdentifierTypeRepr &) = delete;
            void operator=(const IdentifierTypeRepr &) = delete;


            llvm::Expected<sema::Type> resolve(const sema::TypeChecker::State &state) const override;


            llvm::StringRef name() const { return _token->string; }


            static bool classof(const TypeRepr * type) {
                return type->getKind() == Kind::identifier;
            }
        };
    }
}

#endif //JUICE_AST_TYPEREPR_H
