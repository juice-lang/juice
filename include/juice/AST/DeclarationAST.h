// include/juice/AST/DeclarationAST.h - AST nodes for declaration parsing
//
// This source file is part of the juice open source project
//
// Copyright (c) 2019 - 2020 juice project authors
// Licensed under MIT License
//
// See https://github.com/juice-lang/juice/blob/master/LICENSE for license information
// See https://github.com/juice-lang/juice/blob/master/CONTRIBUTORS.txt for the list of juice project authors

#ifndef JUICE_AST_DECLARATIONAST_H
#define JUICE_AST_DECLARATIONAST_H

#include <memory>

#include "ExpressionAST.h"
#include "StatementAST.h"
#include "juice/Parser/LexerToken.h"

namespace juice {
    namespace sema {
        class TypeCheckedVariableDeclarationAST;
    }

    namespace ast {
        class DeclarationAST: public StatementAST {
        protected:
            explicit DeclarationAST(Kind kind): StatementAST(kind) {}

        public:
            DeclarationAST() = delete;

            ~DeclarationAST() override = default;


            static bool classof(const StatementAST * ast) {
                return ast->getKind() >= Kind::declaration
                    && ast->getKind() <= Kind::declaration_last;
            }
        };

        class VariableDeclarationAST: public DeclarationAST {
            std::unique_ptr<parser::LexerToken> _name;
            std::unique_ptr<ExpressionAST> _initialization;

            friend class sema::TypeCheckedVariableDeclarationAST;

        public:
            VariableDeclarationAST() = delete;

            VariableDeclarationAST(std::unique_ptr<parser::LexerToken> name,
                                   std::unique_ptr<ExpressionAST> initialization);

            ~VariableDeclarationAST() override = default;

            basic::SourceLocation getLocation() const override {
                return basic::SourceLocation(_name->string.begin());
            }

            void diagnoseInto(diag::DiagnosticEngine & diagnostics, unsigned int level) const override;

            llvm::Expected<llvm::Value *> codegen(Codegen & state) const override;


            static bool classof(const StatementAST * ast) {
                return ast->getKind() == Kind::variableDeclaration;
            }
        };
    }
}

#endif //JUICE_AST_DECLARATIONAST_H
