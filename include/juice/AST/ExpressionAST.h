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

#include "juice/Parser/LexerToken.h"
#include "juice/Diagnostics/Diagnostics.h"
#include "llvm/IR/IRBuilder.h"
#include "llvm/IR/LLVMContext.h"
#include "llvm/IR/Value.h"

namespace juice {
    namespace ast {
        class ExpressionAST {
        protected:
            std::unique_ptr<parser::LexerToken> _token;

        public:
            explicit ExpressionAST(std::unique_ptr<parser::LexerToken> token);

            virtual ~ExpressionAST() = default;
            virtual void diagnoseInto(diag::DiagnosticEngine & diagnostics, unsigned level = 0) = 0;
            virtual llvm::Value * codegen(llvm::LLVMContext & context, llvm::IRBuilder<> & builder) = 0;
        };

        class BinaryOperatorExpressionAST: public ExpressionAST {
            std::unique_ptr<ExpressionAST> _left, _right;

        public:
            BinaryOperatorExpressionAST(std::unique_ptr<parser::LexerToken> token, std::unique_ptr<ExpressionAST> left,
                                        std::unique_ptr<ExpressionAST> right);

            void diagnoseInto(diag::DiagnosticEngine & diagnostics, unsigned level) override;
            llvm::Value * codegen(llvm::LLVMContext & context, llvm::IRBuilder<> & builder) override;
        };

        class NumberExpressionAST: public ExpressionAST {
            double _value;

        public:
            explicit NumberExpressionAST(std::unique_ptr<parser::LexerToken> token, double value);

            void diagnoseInto(diag::DiagnosticEngine & diagnostics, unsigned level) override;
            llvm::Value * codegen(llvm::LLVMContext & context, llvm::IRBuilder<> & builder) override;
        };

        class GroupingExpressionAST: public ExpressionAST {
            std::unique_ptr<ExpressionAST> _expression;

        public:
            explicit GroupingExpressionAST(std::unique_ptr<parser::LexerToken> token,
                                           std::unique_ptr<ExpressionAST> expression);

            void diagnoseInto(diag::DiagnosticEngine & diagnostics, unsigned level) override;
            llvm::Value * codegen(llvm::LLVMContext & context, llvm::IRBuilder<> & builder) override;
        };
    }
}

#endif //JUICE_EXPRESSIONAST_H
