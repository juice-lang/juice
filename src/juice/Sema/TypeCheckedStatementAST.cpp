// src/juice/Sema/TypeCheckedStatementAST.cpp - type checked statement AST nodes
//
// This source file is part of the juice open source project
//
// Copyright (c) 2019 - 2020 juice project authors
// Licensed under MIT License
//
// See https://github.com/juice-lang/juice/blob/master/LICENSE for license information
// See https://github.com/juice-lang/juice/blob/master/CONTRIBUTORS.txt for the list of juice project authors


#include "juice/Sema/TypeCheckedStatementAST.h"

#include "juice/AST/DeclarationAST.h"
#include "juice/Sema/BuiltinType.h"
#include "juice/Sema/TypeCheckedDeclarationAST.h"
#include "llvm/Support/Casting.h"
#include "llvm/Support/ErrorHandling.h"

namespace juice {
    namespace sema {
        std::unique_ptr<TypeCheckedStatementAST>
        TypeCheckedStatementAST::createByTypeChecking(std::unique_ptr<ast::StatementAST> ast,
                                                      const TypeHint & hint, TypeChecker::State & state,
                                                      diag::DiagnosticEngine & diagnostics) {
            if (llvm::isa<ast::DeclarationAST>(ast.get())) {
                auto declaration = std::unique_ptr<ast::DeclarationAST>(
                    llvm::cast<ast::DeclarationAST>(ast.release()));
                return TypeCheckedDeclarationAST::createByTypeChecking(std::move(declaration), hint, state,
                                                                       diagnostics);
            }

            switch (ast->getKind()) {
                case ast::StatementAST::Kind::block: {
                    auto block = std::unique_ptr<ast::BlockStatementAST>(
                        llvm::cast<ast::BlockStatementAST>(ast.release()));
                    return TypeCheckedBlockStatementAST::createByTypeChecking(std::move(block), hint, state,
                                                                              diagnostics);
                }
                case ast::StatementAST::Kind::expression: {
                    auto block = std::unique_ptr<ast::ExpressionStatementAST>(
                        llvm::cast<ast::ExpressionStatementAST>(ast.release()));
                    return TypeCheckedExpressionStatementAST::createByTypeChecking(std::move(block), hint, state,
                                                                                   diagnostics);
                }
                case ast::StatementAST::Kind::_if: {
                    auto block = std::unique_ptr<ast::IfStatementAST>(
                        llvm::cast<ast::IfStatementAST>(ast.release()));
                    return TypeCheckedIfStatementAST::createByTypeChecking(std::move(block), hint, state, diagnostics);
                }
                case ast::StatementAST::Kind::_while: {
                    auto block = std::unique_ptr<ast::WhileStatementAST>(
                        llvm::cast<ast::WhileStatementAST>(ast.release()));
                    return TypeCheckedWhileStatementAST::createByTypeChecking(std::move(block), hint, state,
                                                                              diagnostics);
                }
                default:
                    llvm_unreachable("All possible Statement kinds should be handled here");
            }
        }

        TypeCheckedBlockStatementAST::TypeCheckedBlockStatementAST(const Type * type,
                                                                   std::unique_ptr<TypeCheckedBlockAST> block):
            TypeCheckedStatementAST(type), _block(std::move(block)) {}

        void
        TypeCheckedBlockStatementAST::diagnoseInto(diag::DiagnosticEngine & diagnostics, unsigned int level) const {
            _block->diagnoseInto(diagnostics, level);
        }

        std::unique_ptr<TypeCheckedBlockStatementAST>
        TypeCheckedBlockStatementAST::createByTypeChecking(std::unique_ptr<ast::BlockStatementAST> ast,
                                                           const TypeHint & hint, TypeChecker::State & state,
                                                           diag::DiagnosticEngine & diagnostics) {
            auto block = TypeCheckedBlockAST::createByTypeChecking(std::move(ast->_block), hint, state, diagnostics);

            auto type = block->getType();

            return std::unique_ptr<TypeCheckedBlockStatementAST>(new TypeCheckedBlockStatementAST(type,
                                                                                                  std::move(block)));
        }

        TypeCheckedExpressionStatementAST
            ::TypeCheckedExpressionStatementAST(const Type * type,
                                                std::unique_ptr<TypeCheckedExpressionAST> expression):
            TypeCheckedStatementAST(type), _expression(std::move(expression)) {}

        void TypeCheckedExpressionStatementAST::diagnoseInto(diag::DiagnosticEngine & diagnostics,
                                                             unsigned int level) const {
            _expression->diagnoseInto(diagnostics, level);
        }

        std::unique_ptr<TypeCheckedExpressionStatementAST>
        TypeCheckedExpressionStatementAST::createByTypeChecking(std::unique_ptr<ast::ExpressionStatementAST> ast,
                                                                const TypeHint & hint, TypeChecker::State & state,
                                                                diag::DiagnosticEngine & diagnostics) {
            auto expression = TypeCheckedExpressionAST::createByTypeChecking(std::move(ast->_expression), hint, state,
                                                                             diagnostics);

            auto type = expression->getType();

            return std::unique_ptr<TypeCheckedExpressionStatementAST>(
                new TypeCheckedExpressionStatementAST(type, std::move(expression)));
        }

        TypeCheckedIfStatementAST::TypeCheckedIfStatementAST(const Type * type,
                                                             std::unique_ptr<TypeCheckedIfExpressionAST> ifExpression):
            TypeCheckedStatementAST(type), _ifExpression(std::move(ifExpression)) {}

        void TypeCheckedIfStatementAST::diagnoseInto(diag::DiagnosticEngine & diagnostics, unsigned int level) const {
            _ifExpression->diagnoseInto(diagnostics, level);
        }

        std::unique_ptr<TypeCheckedIfStatementAST>
        TypeCheckedIfStatementAST::createByTypeChecking(std::unique_ptr<ast::IfStatementAST> ast,
                                                        const TypeHint & hint, TypeChecker::State & state,
                                                        diag::DiagnosticEngine & diagnostics) {
            auto ifExpression = TypeCheckedIfExpressionAST::createByTypeChecking(std::move(ast->_ifExpression), hint,
                                                                                 state, diagnostics);

            auto type = ifExpression->getType();

            return std::unique_ptr<TypeCheckedIfStatementAST>(new TypeCheckedIfStatementAST(type,
                                                                                            std::move(ifExpression)));
        }

        TypeCheckedWhileStatementAST::TypeCheckedWhileStatementAST(const Type * type,
                                                                   std::unique_ptr<TypeCheckedExpressionAST> condition,
                                                                   std::unique_ptr<TypeCheckedControlFlowBodyAST> body):
            TypeCheckedStatementAST(type), _condition(std::move(condition)), _body(std::move(body)) {}

        void
        TypeCheckedWhileStatementAST::diagnoseInto(diag::DiagnosticEngine & diagnostics, unsigned int level) const {
            basic::SourceLocation location(getLocation());

            diagnostics.diagnose(location, diag::DiagnosticID::type_checked_while_statement_ast_0, getColor(level),
                                 getType(), level, _body->getKeyword().get());
            _condition->diagnoseInto(diagnostics, level + 1);

            diagnostics.diagnose(location, diag::DiagnosticID::while_statement_ast_1, getColor(level), level);
            _body->diagnoseInto(diagnostics, level + 1);

            diagnostics.diagnose(location, diag::DiagnosticID::ast_end, getColor(level), level);
        }

        std::unique_ptr<TypeCheckedWhileStatementAST>
        TypeCheckedWhileStatementAST::createByTypeChecking(std::unique_ptr<ast::WhileStatementAST> ast,
                                                           const TypeHint & hint, TypeChecker::State & state,
                                                           diag::DiagnosticEngine & diagnostics) {
            basic::SourceLocation location(ast->getLocation());

            auto condition =
                TypeCheckedExpressionAST::createByTypeChecking(std::move(ast->_condition),
                                                               ExpectedTypeHint(BuiltinIntegerType::getBool()),
                                                               state, diagnostics);

            auto body = TypeCheckedControlFlowBodyAST::createByTypeChecking(std::move(ast->_body), NoneTypeHint(),
                                                                            state, diagnostics);

            switch (hint.getKind()) {
                case TypeHint::Kind::none: break;
                case TypeHint::Kind::unknown:
                    diagnostics.diagnose(location, diag::DiagnosticID::statement_ast_expected_unknown_type);
                    break;
                case TypeHint::Kind::expected: {
                    auto expectedType = llvm::cast<ExpectedTypeHint>(hint).getType();
                    diagnostics.diagnose(location, diag::DiagnosticID::statement_ast_expected_type, expectedType);
                    break;
                }
            }

            return std::unique_ptr<TypeCheckedWhileStatementAST>(
                new TypeCheckedWhileStatementAST(new NothingType, std::move(condition), std::move(body)));
        }
    }
}
