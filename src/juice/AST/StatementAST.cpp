// src/juice/AST/StatementAST.cpp - AST nodes for statement parsing
//
// This source file is part of the juice open source project
//
// Copyright (c) 2019 juice project authors
// Licensed under MIT License
//
// See https://github.com/juice-lang/juice/blob/master/LICENSE for license information
// See https://github.com/juice-lang/juice/blob/master/CONTRIBUTORS.txt for the list of juice project authors

#include "juice/AST/StatementAST.h"

#include <utility>

namespace juice {
    namespace ast {
        ExpressionStatementAST::ExpressionStatementAST(std::unique_ptr<ExpressionAST> expression):
                _expression(std::move(expression)) {}

        void ExpressionStatementAST::diagnoseInto(diag::DiagnosticEngine & diagnostics, unsigned level) const {
            _expression->diagnoseInto(diagnostics, level);
        }

        llvm::Value * ExpressionStatementAST::codegen(llvm::LLVMContext & context, llvm::IRBuilder<> & builder) const {
            return _expression->codegen(context, builder);
        }
    }
}
