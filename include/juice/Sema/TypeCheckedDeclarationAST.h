// include/juice/Sema/TypeCheckedDeclarationAST.h - type checked declaration statement AST nodes
//
// This source file is part of the juice open source project
//
// Copyright (c) 2019 - 2020 juice project authors
// Licensed under MIT License
//
// See https://github.com/juice-lang/juice/blob/master/LICENSE for license information
// See https://github.com/juice-lang/juice/blob/master/CONTRIBUTORS.txt for the list of juice project authors


#ifndef JUICE_SEMA_TYPECHECKEDDECLARATIONAST_H
#define JUICE_SEMA_TYPECHECKEDDECLARATIONAST_H

#include <memory>

#include "TypeCheckedExpressionAST.h"
#include "TypeCheckedStatementAST.h"
#include "TypeChecker.h"
#include "juice/AST/DeclarationAST.h"
#include "juice/Parser/LexerToken.h"

namespace juice {
    namespace sema {
        class TypeCheckedDeclarationAST: public TypeCheckedStatementAST {
        protected:
            explicit TypeCheckedDeclarationAST(Kind kind, Type type): TypeCheckedStatementAST(kind, type) {}

        public:
            TypeCheckedDeclarationAST() = delete;

            ~TypeCheckedDeclarationAST() override = default;

            static std::unique_ptr<TypeCheckedDeclarationAST>
            createByTypeChecking(std::unique_ptr<ast::DeclarationAST> ast, const TypeHint & hint,
                                 TypeChecker::State & state, diag::DiagnosticEngine & diagnostics);


            static bool classof(const TypeCheckedAST * type) {
                return type->getKind() >= Kind::declaration
                    && type->getKind() <= Kind::declaration_last;
            }
        };

        class TypeCheckedVariableDeclarationAST: public TypeCheckedDeclarationAST {
            std::unique_ptr<parser::LexerToken> _keyword, _name;
            std::unique_ptr<TypeCheckedExpressionAST> _initialization;
            size_t _index;

            TypeCheckedVariableDeclarationAST(std::unique_ptr<parser::LexerToken> keyword,
                                              std::unique_ptr<parser::LexerToken> name,
                                              std::unique_ptr<TypeCheckedExpressionAST> initialization, size_t index);

        public:
            TypeCheckedVariableDeclarationAST() = delete;

            ~TypeCheckedVariableDeclarationAST() override = default;

            basic::SourceLocation getLocation() const override {
                return basic::SourceLocation(_keyword->string.begin());
            }

            void diagnoseInto(diag::DiagnosticEngine & diagnostics, unsigned int level) const override;

            static std::unique_ptr<TypeCheckedVariableDeclarationAST>
            createByTypeChecking(std::unique_ptr<ast::VariableDeclarationAST> ast, const TypeHint & hint,
                                 TypeChecker::State & state, diag::DiagnosticEngine & diagnostics);


            static bool classof(const TypeCheckedAST * type) {
                return type->getKind() == Kind::variableDeclaration;
            }
        };
    }
}

#endif //JUICE_SEMA_TYPECHECKEDDECLARATIONAST_H
