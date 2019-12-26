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
        class StatementAST: public AST {
        public:
            void diagnoseInto(diag::DiagnosticEngine & diagnostics, unsigned level) const override = 0;
            llvm::Value * codegen(llvm::LLVMContext & context, llvm::IRBuilder<> & builder) const override = 0;
        };

        class ExpressionStatementAST: public StatementAST {
            std::unique_ptr<ExpressionAST> _expression;

        public:
            ExpressionStatementAST() = delete;

            explicit ExpressionStatementAST(std::unique_ptr<ExpressionAST> expression);

            ~ExpressionStatementAST() override = default;

            void diagnoseInto(diag::DiagnosticEngine & diagnostics, unsigned level) const override;
            llvm::Value * codegen(llvm::LLVMContext & context, llvm::IRBuilder<> & builder) const override;
        };
    }
}

#endif //JUICE_STATEMENTAST_H
