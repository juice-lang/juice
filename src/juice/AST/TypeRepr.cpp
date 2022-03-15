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

#include "llvm/Support/Casting.h"

namespace juice {
    namespace ast {
        IdentifierTypeRepr::IdentifierTypeRepr(std::unique_ptr<parser::LexerToken> token):
            TypeRepr(Kind::identifier), _token(std::move(token)) {}

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
