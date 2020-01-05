// include/juice/Parser/ExpressionAST.h - AST nodes for expression parsing
//
// This source file is part of the juice open source project
//
// Copyright (c) 2019 juice project authors
// Licensed under MIT License
//
// See https://github.com/juice-lang/juice/blob/master/LICENSE for license information
// See https://github.com/juice-lang/juice/blob/master/CONTRIBUTORS.txt for the list of juice project authors


#ifndef JUICE_EXPRESSIONAST_H
#define JUICE_EXPRESSIONAST_H

#include <memory>

#include "AST.h"
#include "juice/Parser/LexerToken.h"
#include "juice/Diagnostics/Diagnostics.h"
#include "llvm/IR/IRBuilder.h"
#include "llvm/IR/LLVMContext.h"
#include "llvm/IR/Value.h"

namespace juice {
    namespace ast {
        class ExpressionAST: public AST {
        protected:
            std::unique_ptr<parser::LexerToken> _token;

        public:
            ExpressionAST() = delete;

            explicit ExpressionAST(std::unique_ptr<parser::LexerToken> token);

            ~ExpressionAST() override = default;

            void diagnoseInto(diag::DiagnosticEngine & diagnostics, unsigned level) const override = 0;

            llvm::Value * codegen(Codegen & state) const override = 0;
        };

        class BinaryOperatorExpressionAST: public ExpressionAST {
            std::unique_ptr<ExpressionAST> _left, _right;

        public:
            BinaryOperatorExpressionAST() = delete;

            BinaryOperatorExpressionAST(std::unique_ptr<parser::LexerToken> token, std::unique_ptr<ExpressionAST> left,
                                        std::unique_ptr<ExpressionAST> right);

            void diagnoseInto(diag::DiagnosticEngine & diagnostics, unsigned level) const override;

            llvm::Value * codegen(Codegen & state) const override;
        };

        class NumberExpressionAST: public ExpressionAST {
            double _value;

        public:
            NumberExpressionAST() = delete;

            NumberExpressionAST(std::unique_ptr<parser::LexerToken> token, double value);

            void diagnoseInto(diag::DiagnosticEngine & diagnostics, unsigned level) const override;

            llvm::Value * codegen(Codegen & state) const override;
        };

        class VariableExpressionAST: public ExpressionAST {
        public:
            VariableExpressionAST() = delete;

            explicit VariableExpressionAST(std::unique_ptr<parser::LexerToken> token);

            void diagnoseInto(diag::DiagnosticEngine &diagnostics, unsigned int level) const override;

            llvm::Value * codegen(Codegen &state) const override;
        };

        class GroupingExpressionAST: public ExpressionAST {
            std::unique_ptr<ExpressionAST> _expression;

        public:
            GroupingExpressionAST() = delete;

            explicit GroupingExpressionAST(std::unique_ptr<parser::LexerToken> token,
                                           std::unique_ptr<ExpressionAST> expression);

            void diagnoseInto(diag::DiagnosticEngine & diagnostics, unsigned level) const override;

            llvm::Value * codegen(Codegen & state) const override;
        };
    }
}

#endif //JUICE_EXPRESSIONAST_H
