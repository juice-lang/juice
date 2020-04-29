// src/juice/Sema/TypeCheckedExpressionAST.cpp - type checked expression AST nodes
//
// This source file is part of the juice open source project
//
// Copyright (c) 2019 - 2020 juice project authors
// Licensed under MIT License
//
// See https://github.com/juice-lang/juice/blob/master/LICENSE for license information
// See https://github.com/juice-lang/juice/blob/master/CONTRIBUTORS.txt for the list of juice project authors


#include "juice/Sema/TypeCheckedExpressionAST.h"

#include <algorithm>
#include <iterator>
#include <tuple>
#include <utility>

#include "juice/Sema/BuiltinType.h"
#include "llvm/Support/Casting.h"
#include "llvm/Support/ErrorHandling.h"

namespace juice {
    namespace sema {
        TypeCheckedExpressionAST::TypeCheckedExpressionAST(const Type * type, TypeCheckedExpressionAST::Kind kind,
                                                           std::unique_ptr<parser::LexerToken> token):
            TypeCheckedAST(type), _kind(kind), _token(std::move(token)) {}

        std::unique_ptr<TypeCheckedExpressionAST>
        TypeCheckedExpressionAST::createByTypeChecking(std::unique_ptr<ast::ExpressionAST> ast, const TypeHint & hint,
                                                       diag::DiagnosticEngine & diagnostics) {
            switch (ast->getKind()) {
                case ast::ExpressionAST::Kind::binaryOperator: {
                    auto binaryOperator = std::unique_ptr<ast::BinaryOperatorExpressionAST>(
                        llvm::cast<ast::BinaryOperatorExpressionAST>(ast.release()));
                    return TypeCheckedBinaryOperatorExpressionAST::createByTypeChecking(std::move(binaryOperator), hint,
                                                                                        diagnostics);
                }
                case ast::ExpressionAST::Kind::number: {
                    auto number = std::unique_ptr<ast::NumberExpressionAST>(
                        llvm::cast<ast::NumberExpressionAST>(ast.release()));
                    return TypeCheckedNumberExpressionAST::createByTypeChecking(std::move(number), hint,
                                                                                diagnostics);
                }
                case ast::ExpressionAST::Kind::variable: {
                    auto variable = std::unique_ptr<ast::VariableExpressionAST>(
                        llvm::cast<ast::VariableExpressionAST>(ast.release()));
                    return TypeCheckedVariableExpressionAST::createByTypeChecking(std::move(variable), hint,
                                                                                  diagnostics);
                }
                case ast::ExpressionAST::Kind::grouping: {
                    auto grouping = std::unique_ptr<ast::GroupingExpressionAST>(
                        llvm::cast<ast::GroupingExpressionAST>(ast.release()));
                    return TypeCheckedGroupingExpressionAST::createByTypeChecking(std::move(grouping), hint,
                                                                                  diagnostics);
                }
                case ast::ExpressionAST::Kind::_if: {
                    auto _if = std::unique_ptr<ast::IfExpressionAST>(llvm::cast<ast::IfExpressionAST>(ast.release()));
                    return TypeCheckedIfExpressionAST::createByTypeChecking(std::move(_if), hint,
                                                                            diagnostics);
                }
            }
        }

        TypeCheckedBinaryOperatorExpressionAST
            ::TypeCheckedBinaryOperatorExpressionAST(const Type * type, std::unique_ptr<parser::LexerToken> token,
                                                     std::unique_ptr<TypeCheckedExpressionAST> left,
                                                     std::unique_ptr<TypeCheckedExpressionAST> right):
            TypeCheckedExpressionAST(type, Kind::binaryOperator, std::move(token)), _left(std::move(left)),
            _right(std::move(right)) {}

        std::unique_ptr<TypeCheckedBinaryOperatorExpressionAST>
        TypeCheckedBinaryOperatorExpressionAST
            ::createByTypeChecking(std::unique_ptr<ast::BinaryOperatorExpressionAST> ast, const TypeHint & hint,
                                   diag::DiagnosticEngine & diagnostics) {
            using TokenType = parser::LexerToken::Type;

            basic::SourceLocation location(ast->getLocation());

            if (hint.requiresLValue()) {
                switch (hint.getKind()) {
                    case TypeHint::Kind::none: break;
                    case TypeHint::Kind::unknown: {
                        diagnostics.diagnose(location, diag::DiagnosticID::expression_ast_expected_lvalue_unknown_type,
                                             "binary operator expression");
                        break;
                    }
                    case TypeHint::Kind::expected: {
                        auto expectedType = llvm::cast<ExpectedTypeHint>(hint).getType();
                        diagnostics.diagnose(location, diag::DiagnosticID::expression_ast_expected_lvalue,
                                             expectedType, "binary operator expression");
                        break;
                    }
                }
            }

            switch (ast->_token->type) {
                case TokenType::operatorEqual:
                case TokenType::operatorPlusEqual:
                case TokenType::operatorMinusEqual:
                case TokenType::operatorAsteriskEqual:
                case TokenType::operatorSlashEqual: {
                    auto left = TypeCheckedExpressionAST
                        ::createByTypeChecking(std::move(ast->_left),
                                               ExpectedTypeHint(BuiltinFloatingPointType::createDouble(),
                                                                TypeHint::Flags::lValue),
                                           diagnostics);

                    auto right = TypeCheckedExpressionAST
                        ::createByTypeChecking(std::move(ast->_right),
                                               ExpectedTypeHint(BuiltinFloatingPointType::createDouble()), diagnostics);

                    const Type * type = BuiltinFloatingPointType::createDouble();

                    switch (hint.getKind()) {
                        case TypeHint::Kind::none:
                        case TypeHint::Kind::unknown: break;
                        case TypeHint::Kind::expected: {
                            auto expectedType = llvm::cast<ExpectedTypeHint>(hint).getType();
                            if (!llvm::isa<BuiltinFloatingPointType>(expectedType)
                                || llvm::cast<BuiltinFloatingPointType>(expectedType)->getFPKind()
                                != BuiltinFloatingPointType::FPKind::ieee64) {
                                diagnostics.diagnose(location, diag::DiagnosticID::expression_ast_expected_type,
                                                     expectedType, type);
                            }
                            break;
                        }
                    }

                    return std::unique_ptr<TypeCheckedBinaryOperatorExpressionAST>(
                        new TypeCheckedBinaryOperatorExpressionAST(type, std::move(ast->_token), std::move(left),
                                                                   std::move(right)));
                }
                case TokenType::operatorAndAnd:
                case TokenType::operatorPipePipe: {
                    auto left = TypeCheckedExpressionAST
                        ::createByTypeChecking(std::move(ast->_left),
                                               ExpectedTypeHint(BuiltinIntegerType::createBool()), diagnostics);

                    auto right = TypeCheckedExpressionAST
                        ::createByTypeChecking(std::move(ast->_right),
                                               ExpectedTypeHint(BuiltinIntegerType::createBool()), diagnostics);

                    const Type * type = BuiltinIntegerType::createBool();

                    switch (hint.getKind()) {
                        case TypeHint::Kind::none:
                        case TypeHint::Kind::unknown: break;
                        case TypeHint::Kind::expected: {
                            auto expectedType = llvm::cast<ExpectedTypeHint>(hint).getType();
                            if (!llvm::isa<BuiltinIntegerType>(expectedType)
                                || llvm::cast<BuiltinIntegerType>(expectedType)->getWidth()
                                    != BuiltinIntegerType::Width::_1) {
                                diagnostics.diagnose(location, diag::DiagnosticID::expression_ast_expected_type,
                                                     expectedType, type);
                            }
                            break;
                        }
                    }

                    return std::unique_ptr<TypeCheckedBinaryOperatorExpressionAST>(
                        new TypeCheckedBinaryOperatorExpressionAST(type, std::move(ast->_token), std::move(left),
                                                                   std::move(right)));
                }
                case TokenType::operatorPlus:
                case TokenType::operatorMinus:
                case TokenType::operatorAsterisk:
                case TokenType::operatorSlash: {
                    auto left = TypeCheckedExpressionAST
                        ::createByTypeChecking(std::move(ast->_left),
                                               ExpectedTypeHint(BuiltinFloatingPointType::createDouble()), diagnostics);

                    auto right = TypeCheckedExpressionAST
                        ::createByTypeChecking(std::move(ast->_right),
                                               ExpectedTypeHint(BuiltinFloatingPointType::createDouble()), diagnostics);

                    const Type * type = BuiltinFloatingPointType::createDouble();

                    switch (hint.getKind()) {
                        case TypeHint::Kind::none:
                        case TypeHint::Kind::unknown: break;
                        case TypeHint::Kind::expected: {
                            auto expectedType = llvm::cast<ExpectedTypeHint>(hint).getType();
                            if (!llvm::isa<BuiltinFloatingPointType>(expectedType)
                                || llvm::cast<BuiltinFloatingPointType>(expectedType)->getFPKind()
                                    != BuiltinFloatingPointType::FPKind::ieee64) {
                                diagnostics.diagnose(location, diag::DiagnosticID::expression_ast_expected_type,
                                                     expectedType, type);
                            }
                            break;
                        }
                    }

                    return std::unique_ptr<TypeCheckedBinaryOperatorExpressionAST>(
                        new TypeCheckedBinaryOperatorExpressionAST(type, std::move(ast->_token), std::move(left),
                                                                   std::move(right)));
                }
                case TokenType::operatorEqualEqual:
                case TokenType::operatorBangEqual: {
                    auto left = TypeCheckedExpressionAST
                        ::createByTypeChecking(std::move(ast->_left), UnknownTypeHint(), diagnostics);

                    const Type * leftType = left->getType();

                    switch (leftType->getKind()) {
                        case Type::Kind::builtinInteger: {
                            if (llvm::cast<BuiltinIntegerType>(leftType)->getWidth() != BuiltinIntegerType::Width::_1) {
                                diagnostics.diagnose(left->getLocation(),
                                                     diag::DiagnosticID::expression_ast_expected_either,
                                                     BuiltinFloatingPointType::createDouble(),
                                                     new BuiltinIntegerType(BuiltinIntegerType::Width::_1), leftType);
                            }
                            break;
                        }
                        case Type::Kind::builtinFloatingPoint: {
                            if (llvm::cast<BuiltinFloatingPointType>(leftType)->getFPKind()
                                != BuiltinFloatingPointType::FPKind::ieee64) {
                                diagnostics.diagnose(left->getLocation(),
                                                     diag::DiagnosticID::expression_ast_expected_either,
                                                     BuiltinFloatingPointType::createDouble(),
                                                     new BuiltinIntegerType(BuiltinIntegerType::Width::_1), leftType);
                            }
                            break;
                        }
                        default: {
                            diagnostics.diagnose(left->getLocation(),
                                                 diag::DiagnosticID::expression_ast_expected_either,
                                                 BuiltinFloatingPointType::createDouble(),
                                                 new BuiltinIntegerType(BuiltinIntegerType::Width::_1), leftType);
                            break;
                        }
                    }

                    auto right = TypeCheckedExpressionAST
                        ::createByTypeChecking(std::move(ast->_right), ExpectedTypeHint(leftType), diagnostics);


                    const Type * type = BuiltinIntegerType::createBool();

                    switch (hint.getKind()) {
                        case TypeHint::Kind::none:
                        case TypeHint::Kind::unknown: break;
                        case TypeHint::Kind::expected: {
                            auto expectedType = llvm::cast<ExpectedTypeHint>(hint).getType();
                            if (!llvm::isa<BuiltinIntegerType>(expectedType)
                                || llvm::cast<BuiltinIntegerType>(expectedType)->getWidth()
                                    != BuiltinIntegerType::Width::_1) {
                                diagnostics.diagnose(location, diag::DiagnosticID::expression_ast_expected_type,
                                                     expectedType, type);
                            }
                            break;
                        }
                    }

                    return std::unique_ptr<TypeCheckedBinaryOperatorExpressionAST>(
                        new TypeCheckedBinaryOperatorExpressionAST(type, std::move(ast->_token), std::move(left),
                                                                   std::move(right)));
                }
                case TokenType::operatorLower:
                case TokenType::operatorLowerEqual:
                case TokenType::operatorGreater:
                case TokenType::operatorGreaterEqual: {
                    auto left = TypeCheckedExpressionAST
                        ::createByTypeChecking(std::move(ast->_left),
                                               ExpectedTypeHint(BuiltinFloatingPointType::createDouble()), diagnostics);

                    auto right = TypeCheckedExpressionAST
                        ::createByTypeChecking(std::move(ast->_right),
                                               ExpectedTypeHint(BuiltinFloatingPointType::createDouble()), diagnostics);

                    const Type * type = BuiltinIntegerType::createBool();

                    switch (hint.getKind()) {
                        case TypeHint::Kind::none:
                        case TypeHint::Kind::unknown: break;
                        case TypeHint::Kind::expected: {
                            auto expectedType = llvm::cast<ExpectedTypeHint>(hint).getType();
                            if (!llvm::isa<BuiltinIntegerType>(expectedType)
                                || llvm::cast<BuiltinIntegerType>(expectedType)->getWidth()
                                    != BuiltinIntegerType::Width::_1) {
                                diagnostics.diagnose(location, diag::DiagnosticID::expression_ast_expected_type,
                                                     expectedType, type);
                            }
                            break;
                        }
                    }

                    return std::unique_ptr<TypeCheckedBinaryOperatorExpressionAST>(
                        new TypeCheckedBinaryOperatorExpressionAST(type, std::move(ast->_token), std::move(left),
                                                                   std::move(right)));
                }
                default:
                    llvm_unreachable("All possible binary operators should be handled here");
            }
        }

        TypeCheckedNumberExpressionAST::TypeCheckedNumberExpressionAST(const Type * type,
                                                                       std::unique_ptr<parser::LexerToken> token,
                                                                       double value):
            TypeCheckedExpressionAST(type, Kind::number, std::move(token)), _value(value) {}

        std::unique_ptr<TypeCheckedNumberExpressionAST>
        TypeCheckedNumberExpressionAST::createByTypeChecking(std::unique_ptr<ast::NumberExpressionAST> ast,
                                                             const TypeHint & hint,
                                                             diag::DiagnosticEngine & diagnostics) {
            basic::SourceLocation location(ast->getLocation());

            auto type = BuiltinFloatingPointType::createDouble();

            if (hint.requiresLValue()) {
                switch (hint.getKind()) {
                    case TypeHint::Kind::none: break;
                    case TypeHint::Kind::unknown: {
                        diagnostics.diagnose(location, diag::DiagnosticID::expression_ast_expected_lvalue_unknown_type,
                                             "number literal");
                        break;
                    }
                    case TypeHint::Kind::expected: {
                        auto expectedType = llvm::cast<ExpectedTypeHint>(hint).getType();
                        diagnostics.diagnose(location, diag::DiagnosticID::expression_ast_expected_lvalue,
                                             expectedType, "number literal");
                        break;
                    }
                }
            }

            switch (hint.getKind()) {
                case TypeHint::Kind::none:
                case TypeHint::Kind::unknown: break;
                case TypeHint::Kind::expected: {
                    auto expectedType = llvm::cast<ExpectedTypeHint>(hint).getType();
                    if (!llvm::isa<BuiltinFloatingPointType>(expectedType)
                        || llvm::cast<BuiltinFloatingPointType>(expectedType)->getFPKind()
                            != BuiltinFloatingPointType::FPKind::ieee64) {
                        diagnostics.diagnose(location, diag::DiagnosticID::expression_ast_expected_type,
                                             expectedType, type);
                    }
                    break;
                }
            }

            return std::unique_ptr<TypeCheckedNumberExpressionAST>(
                new TypeCheckedNumberExpressionAST(type, std::move(ast->_token), ast->_value));
        }

        TypeCheckedVariableExpressionAST::TypeCheckedVariableExpressionAST(const Type * type,
                                                                           std::unique_ptr<parser::LexerToken> token):
            TypeCheckedExpressionAST(type, Kind::variable, std::move(token)) {}

        std::unique_ptr<TypeCheckedVariableExpressionAST>
        TypeCheckedVariableExpressionAST::createByTypeChecking(std::unique_ptr<ast::VariableExpressionAST> ast,
                                                               const TypeHint & hint,
                                                               diag::DiagnosticEngine & diagnostics) {
            basic::SourceLocation location(ast->getLocation());

            auto type = BuiltinFloatingPointType::createDouble();

            switch (hint.getKind()) {
                case TypeHint::Kind::none:
                case TypeHint::Kind::unknown: break;
                case TypeHint::Kind::expected: {
                    auto expectedType = llvm::cast<ExpectedTypeHint>(hint).getType();
                    if (!llvm::isa<BuiltinFloatingPointType>(expectedType)
                        || llvm::cast<BuiltinFloatingPointType>(expectedType)->getFPKind()
                            != BuiltinFloatingPointType::FPKind::ieee64) {
                        diagnostics.diagnose(location, diag::DiagnosticID::expression_ast_expected_type, expectedType,
                                             type);
                    }
                    break;
                }
            }

            return std::unique_ptr<TypeCheckedVariableExpressionAST>(
                new TypeCheckedVariableExpressionAST(type, std::move(ast->_token)));
        }

        TypeCheckedGroupingExpressionAST
            ::TypeCheckedGroupingExpressionAST(const Type * type, std::unique_ptr<parser::LexerToken> token,
                                               std::unique_ptr<TypeCheckedExpressionAST> expression):
            TypeCheckedExpressionAST(type, Kind::grouping, std::move(token)), _expression(std::move(expression)) {}

        std::unique_ptr<TypeCheckedGroupingExpressionAST>
        TypeCheckedGroupingExpressionAST::createByTypeChecking(std::unique_ptr<ast::GroupingExpressionAST> ast,
                                                               const TypeHint & hint,
                                                               diag::DiagnosticEngine & diagnostics) {
            auto expression = TypeCheckedExpressionAST::createByTypeChecking(std::move(ast->_expression), hint,
                                                                             diagnostics);

            auto type = expression->getType();

            return std::unique_ptr<TypeCheckedGroupingExpressionAST>(
                new TypeCheckedGroupingExpressionAST(type, std::move(ast->_token), std::move(expression)));
        }

        TypeCheckedIfExpressionAST
            ::TypeCheckedIfExpressionAST(const Type * type, std::unique_ptr<TypeCheckedExpressionAST> ifCondition,
                                         std::unique_ptr<TypeCheckedControlFlowBodyAST> ifBody,
                                         ElifVector && elifConditionsAndBodies,
                                         std::unique_ptr<TypeCheckedControlFlowBodyAST> elseBody, bool isStatement):
            TypeCheckedExpressionAST(type, Kind::_if, nullptr), _ifCondition(std::move(ifCondition)),
            _ifBody(std::move(ifBody)), _elifConditionsAndBodies(std::move(elifConditionsAndBodies)),
            _elseBody(std::move(elseBody)), _isStatement(isStatement) {}

        std::unique_ptr<TypeCheckedIfExpressionAST>
        TypeCheckedIfExpressionAST::createByTypeChecking(std::unique_ptr<ast::IfExpressionAST> ast,
                                                         const TypeHint & hint, diag::DiagnosticEngine & diagnostics) {
            basic::SourceLocation location(ast->getLocation());

            if (hint.requiresLValue()) {
                switch (hint.getKind()) {
                    case TypeHint::Kind::none: break;
                    case TypeHint::Kind::unknown: {
                        diagnostics.diagnose(location, diag::DiagnosticID::expression_ast_expected_lvalue_unknown_type,
                                             ast->_isStatement ? "if statement" : "if expression");
                        break;
                    }
                    case TypeHint::Kind::expected: {
                        auto expectedType = llvm::cast<ExpectedTypeHint>(hint).getType();
                        diagnostics.diagnose(location, diag::DiagnosticID::expression_ast_expected_lvalue,
                                             expectedType, ast->_isStatement ? "if statement" : "if expression");
                        break;
                    }
                }
            }

            const TypeHint & noneTypeHint = NoneTypeHint();
            const TypeHint & ifHint = ast->_isStatement ? noneTypeHint : hint;

            auto ifCondition =
                TypeCheckedExpressionAST::createByTypeChecking(std::move(ast->_ifCondition),
                                                               ExpectedTypeHint(BuiltinIntegerType::createBool()),
                                                               diagnostics);

            auto ifBody = TypeCheckedControlFlowBodyAST::createByTypeChecking(std::move(ast->_ifBody), ifHint,
                                                                              diagnostics);

            auto type = ast->_isStatement ? new NothingType : ifBody->getType();

            ElifVector elifConditionsAndBodies;

            std::transform(std::make_move_iterator(ast->_elifConditionsAndBodies.begin()),
                           std::make_move_iterator(ast->_elifConditionsAndBodies.end()),
                           std::back_inserter(elifConditionsAndBodies),
                           [&ifHint, &diagnostics](std::pair<std::unique_ptr<ast::ExpressionAST>,
                                                           std::unique_ptr<ast::ControlFlowBodyAST>> conditionAndBody) {
                std::unique_ptr<ast::ExpressionAST> condition;
                std::unique_ptr<ast::ControlFlowBodyAST> body;
                std::tie(condition, body) = std::move(conditionAndBody);

                auto checkedCondition =
                    TypeCheckedExpressionAST::createByTypeChecking(std::move(condition),
                                                                   ExpectedTypeHint(BuiltinIntegerType::createBool()),
                                                                   diagnostics);

                auto checkedBody = TypeCheckedControlFlowBodyAST::createByTypeChecking(std::move(body), ifHint,
                                                                                       diagnostics);

                return std::make_pair(std::move(checkedCondition), std::move(checkedBody));
            });

            std::unique_ptr<TypeCheckedControlFlowBodyAST> elseBody;

            if (!ast->_isStatement || ast->_elseBody) {
                elseBody = TypeCheckedControlFlowBodyAST::createByTypeChecking(std::move(ast->_elseBody), ifHint,
                                                                                    diagnostics);
            }

            return std::unique_ptr<TypeCheckedIfExpressionAST>(
                new TypeCheckedIfExpressionAST(type, std::move(ifCondition), std::move(ifBody),
                                               std::move(elifConditionsAndBodies), std::move(elseBody),
                                               ast->_isStatement));
        }
    }
}
