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

#include "juice/AST/Codegen.h"

namespace juice {
    namespace ast {
        BlockStatementAST::BlockStatementAST(std::unique_ptr<BlockAST> block): _block(std::move(block)) {}

        void BlockStatementAST::diagnoseInto(diag::DiagnosticEngine & diagnostics, unsigned int level) const {
            _block->diagnoseInto(diagnostics, level);
        }

        llvm::Expected<llvm::Value *> BlockStatementAST::codegen(Codegen & state) const {
            return _block->codegen(state);
        }

        ExpressionStatementAST::ExpressionStatementAST(std::unique_ptr<ExpressionAST> expression):
                _expression(std::move(expression)) {}

        void ExpressionStatementAST::diagnoseInto(diag::DiagnosticEngine & diagnostics, unsigned int level) const {
            _expression->diagnoseInto(diagnostics, level);
        }

        llvm::Expected<llvm::Value *> ExpressionStatementAST::codegen(Codegen & state) const {
            return _expression->codegen(state);
        }
    }
}
