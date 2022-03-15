// src/juice/Parser/ExpressionAST.cpp - AST nodes for expression parsing
//
// This source file is part of the juice open source project
//
// Copyright (c) 2019 - 2020 juice project authors
// Licensed under MIT License
//
// See https://github.com/juice-lang/juice/blob/master/LICENSE for license information
// See https://github.com/juice-lang/juice/blob/master/CONTRIBUTORS.txt for the list of juice project authors


#include "juice/AST/ExpressionAST.h"

#include <utility>

namespace juice {
    namespace ast {
        ExpressionAST::ExpressionAST(Kind kind, std::unique_ptr<juice::parser::LexerToken> token):
            _kind(kind), _token(std::move(token)) {}

        BinaryOperatorExpressionAST::BinaryOperatorExpressionAST(std::unique_ptr<parser::LexerToken> token,
                                                                 std::unique_ptr<ExpressionAST> left,
                                                                 std::unique_ptr<ExpressionAST> right):
            ExpressionAST(Kind::binaryOperator, std::move(token)), _left(std::move(left)), _right(std::move(right)) {}

        void BinaryOperatorExpressionAST::diagnoseInto(diag::DiagnosticEngine & diagnostics, unsigned int level) const {
            basic::SourceLocation location(getLocation());

            diagnostics.diagnose(location, diag::DiagnosticID::binary_operator_expression_ast_0, getColor(level),
                                 level, _token.get());
            _left->diagnoseInto(diagnostics, level + 1);

            diagnostics
                .diagnose(location, diag::DiagnosticID::binary_operator_expression_ast_1, getColor(level), level);
            _right->diagnoseInto(diagnostics, level + 1);

            diagnostics.diagnose(location, diag::DiagnosticID::ast_end, getColor(level), level);
        }

        NumberExpressionAST::NumberExpressionAST(std::unique_ptr<parser::LexerToken> token, double value):
                ExpressionAST(Kind::number, std::move(token)), _value(value) {}

        void NumberExpressionAST::diagnoseInto(diag::DiagnosticEngine & diagnostics, unsigned int level) const {
            diagnostics.diagnose(getLocation(), diag::DiagnosticID::number_expression_ast, getColor(level), level,
                                 _token.get(), _value);
        }

        BooleanLiteralExpressionAST::BooleanLiteralExpressionAST(std::unique_ptr<parser::LexerToken> token, bool value):
            ExpressionAST(Kind::booleanLiteral, std::move(token)), _value(value) {}

        void BooleanLiteralExpressionAST::diagnoseInto(diag::DiagnosticEngine & diagnostics, unsigned int level) const {
            diagnostics.diagnose(getLocation(), diag::DiagnosticID::boolean_literal_expression_ast, getColor(level),
                                 level, _token.get(), _value);
        }

        VariableExpressionAST::VariableExpressionAST(std::unique_ptr<parser::LexerToken> token):
                ExpressionAST(Kind::variable, std::move(token)) {}

        void VariableExpressionAST::diagnoseInto(diag::DiagnosticEngine & diagnostics, unsigned int level) const {
            diagnostics.diagnose(getLocation(), diag::DiagnosticID::variable_expression_ast, getColor(level),
                                 _token.get());
        }

        GroupingExpressionAST::GroupingExpressionAST(std::unique_ptr<parser::LexerToken> token,
                                                     std::unique_ptr<ExpressionAST> expression):
                ExpressionAST(Kind::grouping, std::move(token)), _expression(std::move(expression)) {}

        void GroupingExpressionAST::diagnoseInto(diag::DiagnosticEngine & diagnostics, unsigned int level) const {
            _expression->diagnoseInto(diagnostics, level);
        }

        IfExpressionAST::IfExpressionAST(std::unique_ptr<ExpressionAST> ifCondition,
                                         std::unique_ptr<ControlFlowBodyAST> ifBody,
                                         std::vector<std::pair<std::unique_ptr<ExpressionAST>,
                                                               std::unique_ptr<ControlFlowBodyAST>>> && elifConditionsAndBodies,
                                         std::unique_ptr<ControlFlowBodyAST> elseBody, bool isStatement):
            ExpressionAST(Kind::_if, nullptr), _ifCondition(std::move(ifCondition)), _ifBody(std::move(ifBody)),
            _elifConditionsAndBodies(std::move(elifConditionsAndBodies)), _elseBody(std::move(elseBody)),
            _isStatement(isStatement) {}

        void IfExpressionAST::diagnoseInto(diag::DiagnosticEngine & diagnostics, unsigned int level) const {
            basic::SourceLocation location(getLocation());

            if (_isStatement) diagnostics.diagnose(location, diag::DiagnosticID::if_statement_ast_0, getColor(level),
                                                   level, _ifBody->getKeyword().get());
            else diagnostics.diagnose(location, diag::DiagnosticID::if_expression_ast_0, getColor(level), level,
                                 _ifBody->getKeyword().get());
            _ifCondition->diagnoseInto(diagnostics, level + 1);

            diagnostics.diagnose(location, diag::DiagnosticID::if_ast_1, getColor(level), level);
            _ifBody->diagnoseInto(diagnostics, level + 1);

            for (const auto & conditionAndBody: _elifConditionsAndBodies) {
                const auto & condition = std::get<0>(conditionAndBody);
                const auto & body = std::get<1>(conditionAndBody);

                diagnostics.diagnose(location, diag::DiagnosticID::if_ast_2, getColor(level), level);
                condition->diagnoseInto(diagnostics, level + 1);

                diagnostics.diagnose(location, diag::DiagnosticID::if_ast_3, getColor(level), level);
                body->diagnoseInto(diagnostics, level + 1);
            }

            if (!_isStatement || _elseBody) {
                diagnostics.diagnose(location, diag::DiagnosticID::if_ast_4, getColor(level), level);
                _elseBody->diagnoseInto(diagnostics, level + 1);
            }

            diagnostics.diagnose(location, diag::DiagnosticID::ast_end, getColor(level), level);
        }
    }
}
