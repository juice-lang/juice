// include/juice/AST/StatementAST.h - AST nodes for statement parsing
//
// This source file is part of the juice open source project
//
// Copyright (c) 2019 juice project authors
// Licensed under MIT License
//
// See https://github.com/juice-lang/juice/blob/master/LICENSE for license information
// See https://github.com/juice-lang/juice/blob/master/CONTRIBUTORS.txt for the list of juice project authors


#ifndef JUICE_STATEMENTAST_H
#define JUICE_STATEMENTAST_H

#include <memory>

#include "AST.h"
#include "ExpressionAST.h"

namespace juice {
    namespace ast {
        class StatementAST: public AST {};

        class BlockStatementAST: public StatementAST {
            std::unique_ptr<BlockAST> _block;

        public:
            BlockStatementAST() = delete;

            explicit BlockStatementAST(std::unique_ptr<BlockAST> block);

            ~BlockStatementAST() override = default;

            basic::SourceLocation getLocation() const override {
                return _block->getLocation();
            }

            void diagnoseInto(diag::DiagnosticEngine & diagnostics, unsigned int level) const override;

            llvm::Expected<llvm::Value *> codegen(Codegen & state) const override;
        };

        class ExpressionStatementAST: public StatementAST {
            std::unique_ptr<ExpressionAST> _expression;

        public:
            ExpressionStatementAST() = delete;

            explicit ExpressionStatementAST(std::unique_ptr<ExpressionAST> expression);

            ~ExpressionStatementAST() override = default;

            basic::SourceLocation getLocation() const override {
                return _expression->getLocation();
            }

            void diagnoseInto(diag::DiagnosticEngine & diagnostics, unsigned int level) const override;

            llvm::Expected<llvm::Value *> codegen(Codegen & state) const override;
        };
    }
}

#endif //JUICE_STATEMENTAST_H
