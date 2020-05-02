// src/juice/AST/StatementAST.cpp - AST nodes for statement parsing
//
// This source file is part of the juice open source project
//
// Copyright (c) 2019 - 2020 juice project authors
// Licensed under MIT License
//
// See https://github.com/juice-lang/juice/blob/master/LICENSE for license information
// See https://github.com/juice-lang/juice/blob/master/CONTRIBUTORS.txt for the list of juice project authors

#include "juice/AST/StatementAST.h"

#include <utility>

namespace juice {
    namespace ast {
        BlockStatementAST::BlockStatementAST(std::unique_ptr<BlockAST> block):
            StatementAST(Kind::block), _block(std::move(block)) {}

        void BlockStatementAST::diagnoseInto(diag::DiagnosticEngine & diagnostics, unsigned int level) const {
            _block->diagnoseInto(diagnostics, level);
        }

        ExpressionStatementAST::ExpressionStatementAST(std::unique_ptr<ExpressionAST> expression):
            StatementAST(Kind::expression), _expression(std::move(expression)) {}

        void ExpressionStatementAST::diagnoseInto(diag::DiagnosticEngine & diagnostics, unsigned int level) const {
            _expression->diagnoseInto(diagnostics, level);
        }

        IfStatementAST::IfStatementAST(std::unique_ptr<IfExpressionAST> ifExpression):
            StatementAST(Kind::_if), _ifExpression(std::move(ifExpression)) {}

        void IfStatementAST::diagnoseInto(diag::DiagnosticEngine & diagnostics, unsigned int level) const {
            _ifExpression->diagnoseInto(diagnostics, level);
        }

        WhileStatementAST::WhileStatementAST(std::unique_ptr<ExpressionAST> condition,
                                             std::unique_ptr<ControlFlowBodyAST> body):
            StatementAST(Kind::_while), _condition(std::move(condition)), _body(std::move(body)) {}

        void WhileStatementAST::diagnoseInto(diag::DiagnosticEngine & diagnostics, unsigned int level) const {
            basic::SourceLocation location(getLocation());

            diagnostics.diagnose(location, diag::DiagnosticID::while_statement_ast_0, getColor(level), level,
                                 _body->getKeyword().get());
            _condition->diagnoseInto(diagnostics, level + 1);

            diagnostics.diagnose(location, diag::DiagnosticID::while_statement_ast_1, getColor(level), level);
            _body->diagnoseInto(diagnostics, level + 1);

            diagnostics.diagnose(location, diag::DiagnosticID::ast_end, getColor(level), level);
        }
    }
}
