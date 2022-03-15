// src/juice/AST/TypeRepr.cpp - Representation of a type as written in source
//
// This source file is part of the juice open source project
//
// Copyright (c) 2019 - 2022 juice project authors
// Licensed under MIT License
//
// See https://github.com/juice-lang/juice/blob/master/LICENSE for license information
// See https://github.com/juice-lang/juice/blob/master/CONTRIBUTORS.txt for the list of juice project authors


#include "juice/AST/TypeRepr.h"

#include <utility>

#include "juice/Basic/Error.h"
#include "juice/Basic/SourceLocation.h"
#include "juice/Diagnostics/DiagnosticError.h"
#include "llvm/Support/Casting.h"

namespace juice {
    namespace ast {
        IdentifierTypeRepr::IdentifierTypeRepr(std::unique_ptr<parser::LexerToken> token):
            TypeRepr(Kind::identifier), _token(std::move(token)) {}

        llvm::Expected<sema::Type> IdentifierTypeRepr::resolve(const sema::TypeChecker::State & state) const {
            auto type = state.getTypeDeclaration(name());

            if (!type) {
                basic::SourceLocation location(name().begin());
                auto id = state.hasVariableDeclaration(name()) ? diag::DiagnosticID::not_a_type
                                                               : diag::DiagnosticID::unresolved_identifer;

                return basic::createError<diag::DiagnosticError>(location, id, name());
            }

            return *type;
        }

        llvm::raw_ostream & operator<<(llvm::raw_ostream & os, const TypeRepr * typeRepr) {
            if (typeRepr) {
                switch (typeRepr->getKind()) {
                    case TypeRepr::Kind::identifier: {
                        os << llvm::cast<IdentifierTypeRepr>(typeRepr)->name();
                        break;
                    }
                }
            } else {
                os << "--";
            }

            return os;
        }
    }
}
