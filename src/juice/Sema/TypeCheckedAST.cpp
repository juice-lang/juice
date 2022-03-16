// src/juice/Sema/TypeCheckedAST.cpp - basic type-checked AST nodes
//
// This source file is part of the juice open source project
//
// Copyright (c) 2019 - 2020 juice project authors
// Licensed under MIT License
//
// See https://github.com/juice-lang/juice/blob/master/LICENSE for license information
// See https://github.com/juice-lang/juice/blob/master/CONTRIBUTORS.txt for the list of juice project authors


#include "juice/Sema/TypeCheckedAST.h"

#include <algorithm>
#include <iterator>
#include <utility>

#include "juice/Sema/TypeCheckedStatementAST.h"
#include "llvm/Support/Casting.h"

namespace juice {
    namespace sema {
        TypeCheckedContainerAST
            ::TypeCheckedContainerAST(Kind kind, Type type,
                                      std::vector<std::unique_ptr<TypeCheckedStatementAST>> && statements):
            TypeCheckedAST(kind, type), _statements(std::move(statements)) {}

        basic::SourceLocation TypeCheckedContainerAST::getLocation() const {
            if (!_statements.empty()) return _statements.front()->getLocation();
            return {};
        }

        TypeCheckedModuleAST::TypeCheckedModuleAST(Type type, StatementVector && statements):
            TypeCheckedContainerAST(Kind::module, type, std::move(statements)) {}

        void TypeCheckedModuleAST::diagnoseInto(diag::DiagnosticEngine & diagnostics, unsigned int level) const {
            for (const auto & statement: _statements) { statement->diagnoseInto(diagnostics, level); }
        }

        std::unique_ptr<TypeCheckedModuleAST>
        TypeCheckedModuleAST::createByTypeChecking(std::unique_ptr<ast::ModuleAST> ast, const TypeHint & hint,
                                                   TypeChecker::State & state, diag::DiagnosticEngine & diagnostics) {
            basic::SourceLocation location(ast->getLocation());

            StatementVector statements;

            if (!ast->_statements.empty()) {
                auto statementInserter =
                    std::transform(std::make_move_iterator(ast->_statements.begin()),
                                   std::make_move_iterator(ast->_statements.end() - 1),
                                   std::back_inserter(statements),
                                   [&state, &diagnostics](std::unique_ptr<ast::StatementAST> statement) {
                    return TypeCheckedStatementAST::createByTypeChecking(std::move(statement), NoneTypeHint(),
                                                                         state, diagnostics);
                });

                *statementInserter = TypeCheckedStatementAST::createByTypeChecking(std::move(ast->_statements.back()),
                                                                                   hint, state, diagnostics);

                Type type = statements.back()->getType();

                return std::unique_ptr<TypeCheckedModuleAST>(new TypeCheckedModuleAST(type, std::move(statements)));
            } else {
                if (llvm::isa<ExpectedTypeHint>(hint)) {
                    Type expectedType = llvm::cast<ExpectedTypeHint>(hint).getType();

                    diagnostics.diagnose(location, diag::DiagnosticID::module_ast_expected_type, expectedType);
                } else if (llvm::isa<ExpectedEitherTypeHint>(hint)) {
                    const auto & types = llvm::cast<ExpectedEitherTypeHint>(hint).getTypes();

                    diagnostics.diagnose(location, diag::DiagnosticID::module_ast_expected_types, &types);
                }

                Type type = NothingType::get();

                return std::unique_ptr<TypeCheckedModuleAST>(new TypeCheckedModuleAST(type, std::move(statements)));
            }
        }

        TypeCheckedBlockAST::TypeCheckedBlockAST(Type type, StatementVector && statements,
                                                 std::unique_ptr<parser::LexerToken> start):
            TypeCheckedContainerAST(Kind::block, type, std::move(statements)), _start(std::move(start)) {}

        void TypeCheckedBlockAST::diagnoseInto(diag::DiagnosticEngine & diagnostics, unsigned int level) const {
            basic::SourceLocation location(getLocation());

            if (_statements.empty()) {
                diagnostics.diagnose(location, diag::DiagnosticID::type_checked_block_ast_empty, getColor(level),
                                     getType());
            } else {
                diagnostics.diagnose(location, diag::DiagnosticID::type_checked_block_ast_0, getColor(level), getType(),
                                     level);

                for (const auto & statement: _statements) {
                    diagnostics.diagnose(location, diag::DiagnosticID::block_ast_1, level + 1);

                    statement->diagnoseInto(diagnostics, level + 1);
                }

                diagnostics.diagnose(location, diag::DiagnosticID::block_ast_2, getColor(level), level);
            }
        }

        std::unique_ptr<TypeCheckedBlockAST>
        TypeCheckedBlockAST::createByTypeChecking(std::unique_ptr<ast::BlockAST> ast, const TypeHint & hint,
                                                  TypeChecker::State & state, diag::DiagnosticEngine & diagnostics) {
            basic::SourceLocation location(ast->getLocation());

            StatementVector statements;

            if (!ast->_statements.empty()) {
                state.newScope();

                auto statementInserter =
                    std::transform(std::make_move_iterator(ast->_statements.begin()),
                                   std::make_move_iterator(ast->_statements.end() - 1),
                                   std::back_inserter(statements),
                                   [&state, &diagnostics](std::unique_ptr<ast::StatementAST> statement) {
                    return TypeCheckedStatementAST::createByTypeChecking(std::move(statement), NoneTypeHint(),
                                                                         state, diagnostics);
                });

                *statementInserter = TypeCheckedStatementAST::createByTypeChecking(std::move(ast->_statements.back()),
                                                                                   hint, state, diagnostics);

                Type type = statements.back()->getType();

                state.endScope();

                return std::unique_ptr<TypeCheckedBlockAST>(new TypeCheckedBlockAST(type, std::move(statements),
                                                                                    std::move(ast->_start)));
            } else {
                if (llvm::isa<ExpectedTypeHint>(hint)) {
                    Type expectedType = llvm::cast<ExpectedTypeHint>(hint).getType();

                    diagnostics.diagnose(location, diag::DiagnosticID::block_ast_expected_type, expectedType);
                } else if (llvm::isa<ExpectedEitherTypeHint>(hint)) {
                    const auto & types = llvm::cast<ExpectedEitherTypeHint>(hint).getTypes();

                    diagnostics.diagnose(location, diag::DiagnosticID::block_ast_expected_types, &types);
                }

                Type type = NothingType::get();

                return std::unique_ptr<TypeCheckedBlockAST>(new TypeCheckedBlockAST(type, std::move(statements),
                                                                                    std::move(ast->_start)));
            }
        }

        TypeCheckedControlFlowBodyAST::TypeCheckedControlFlowBodyAST(Type type,
                                                                     std::unique_ptr<parser::LexerToken> keyword,
                                                                     std::unique_ptr<TypeCheckedBlockAST> block):
            TypeCheckedAST(Kind::controlFlowBody, type), _keyword(std::move(keyword)), _bodyKind(BodyKind::block) {
            new (&_block) std::unique_ptr<TypeCheckedBlockAST>(std::move(block));
        }

        TypeCheckedControlFlowBodyAST
            ::TypeCheckedControlFlowBodyAST(Type type, std::unique_ptr<parser::LexerToken> keyword,
                                            std::unique_ptr<TypeCheckedExpressionAST> expression):
            TypeCheckedAST(Kind::controlFlowBody, type), _keyword(std::move(keyword)), _bodyKind(BodyKind::expression) {
            new (&_expression) std::unique_ptr<TypeCheckedExpressionAST>(std::move(expression));
        }

        TypeCheckedControlFlowBodyAST::~TypeCheckedControlFlowBodyAST() {
            switch (_bodyKind) {
                case BodyKind::block: _block.~unique_ptr<TypeCheckedBlockAST>(); break;
                case BodyKind::expression: _expression.~unique_ptr<TypeCheckedExpressionAST>(); break;
            }
        }

        void
        TypeCheckedControlFlowBodyAST::diagnoseInto(diag::DiagnosticEngine & diagnostics, unsigned int level) const {
            basic::SourceLocation location(getLocation());

            switch (_bodyKind) {
                case BodyKind::block:
                    diagnostics.diagnose(location, diag::DiagnosticID::type_checked_if_body_ast_block, getColor(level),
                                         getType(), level, _keyword.get());
                    _block->diagnoseInto(diagnostics, level + 1);
                    break;
                case BodyKind::expression:
                    diagnostics.diagnose(location, diag::DiagnosticID::type_checked_if_body_ast_expression,
                                         getColor(level), getType(), level, _keyword.get());
                    _expression->diagnoseInto(diagnostics, level + 1);
                    break;
            }

            diagnostics.diagnose(location, diag::DiagnosticID::ast_end, getColor(level), level);
        }

        std::unique_ptr<TypeCheckedControlFlowBodyAST>
        TypeCheckedControlFlowBodyAST::createByTypeChecking(std::unique_ptr<ast::ControlFlowBodyAST> ast,
                                                            const TypeHint & hint, TypeChecker::State & state,
                                                            diag::DiagnosticEngine & diagnostics) {
            switch (ast->_kind) {
                case ast::ControlFlowBodyAST::Kind::block: {
                    auto block = TypeCheckedBlockAST::createByTypeChecking(std::move(ast->_block), hint, state,
                                                                           diagnostics);
                    Type type = block->getType();

                    return std::unique_ptr<TypeCheckedControlFlowBodyAST>(
                        new TypeCheckedControlFlowBodyAST(type, std::move(ast->_keyword), std::move(block)));
                }
                case ast::ControlFlowBodyAST::Kind::expression: {
                    auto expression = TypeCheckedExpressionAST::createByTypeChecking(std::move(ast->_expression), hint,
                                                                                     state, diagnostics);
                    Type type = expression->getType();

                    return std::unique_ptr<TypeCheckedControlFlowBodyAST>(
                        new TypeCheckedControlFlowBodyAST(type, std::move(ast->_keyword), std::move(expression)));
                }
            }
        }
    }
}
