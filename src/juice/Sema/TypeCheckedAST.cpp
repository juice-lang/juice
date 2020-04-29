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
            ::TypeCheckedContainerAST(const Type * type,
                                      std::vector<std::unique_ptr<TypeCheckedStatementAST>> && statements):
            TypeCheckedAST(type), _statements(std::move(statements)) {}

        basic::SourceLocation TypeCheckedContainerAST::getLocation() const {
            if (!_statements.empty()) return _statements.front()->getLocation();
            return {};
        }

        TypeCheckedModuleAST::TypeCheckedModuleAST(const Type * type, StatementVector && statements):
            TypeCheckedContainerAST(type, std::move(statements)) {}

        std::unique_ptr<TypeCheckedModuleAST>
        TypeCheckedModuleAST::createByTypeChecking(std::unique_ptr<ast::ModuleAST> ast, const TypeHint & hint,
                                                   diag::DiagnosticEngine & diagnostics) {
            basic::SourceLocation location(ast->getLocation());

            StatementVector statements;
            const Type * type;

            if (!ast->_statements.empty()) {
                auto statementInserter = std::transform(std::make_move_iterator(ast->_statements.begin()),
                                                        std::make_move_iterator(ast->_statements.end() - 1),
                                                        std::back_inserter(statements),
                                                        [&diagnostics](std::unique_ptr<ast::StatementAST> statement) {
                    return TypeCheckedStatementAST::createByTypeChecking(std::move(statement), NoneTypeHint(),
                                                                         diagnostics);
                });

                *statementInserter = TypeCheckedStatementAST::createByTypeChecking(std::move(ast->_statements.back()),
                                                                                   hint, diagnostics);

                type = statements.back()->getType();
            } else {
                if (llvm::isa<ExpectedTypeHint>(hint)) {
                    auto expectedType = llvm::cast<ExpectedTypeHint>(hint).getType();

                    diagnostics.diagnose(location, diag::DiagnosticID::module_ast_expected_type, expectedType);
                }

                type = new NothingType;
            }

            return std::unique_ptr<TypeCheckedModuleAST>(new TypeCheckedModuleAST(type, std::move(statements)));
        }

        TypeCheckedBlockAST::TypeCheckedBlockAST(const Type * type, StatementVector && statements,
                                                 std::unique_ptr<parser::LexerToken> start):
            TypeCheckedContainerAST(type, std::move(statements)), _start(std::move(start)) {}

        std::unique_ptr<TypeCheckedBlockAST>
        TypeCheckedBlockAST::createByTypeChecking(std::unique_ptr<ast::BlockAST> ast, const TypeHint & hint,
                                                  diag::DiagnosticEngine & diagnostics) {
            basic::SourceLocation location(ast->getLocation());

            StatementVector statements;
            const Type * type;

            if (!ast->_statements.empty()) {
                auto statementInserter = std::transform(std::make_move_iterator(ast->_statements.begin()),
                                                        std::make_move_iterator(ast->_statements.end() - 1),
                                                        std::back_inserter(statements),
                                                        [&diagnostics](std::unique_ptr<ast::StatementAST> statement) {
                    return TypeCheckedStatementAST::createByTypeChecking(std::move(statement), NoneTypeHint(),
                                                                         diagnostics);
                });

                *statementInserter = TypeCheckedStatementAST::createByTypeChecking(std::move(ast->_statements.back()),
                                                                                   hint, diagnostics);

                type = statements.back()->getType();
            } else {
                if (llvm::isa<ExpectedTypeHint>(hint)) {
                    auto expectedType = llvm::cast<ExpectedTypeHint>(hint).getType();

                    diagnostics.diagnose(location, diag::DiagnosticID::module_ast_expected_type, expectedType);
                }

                type = new NothingType;
            }

            return std::unique_ptr<TypeCheckedBlockAST>(new TypeCheckedBlockAST(type, std::move(statements),
                                                                                std::move(ast->_start)));
        }

        TypeCheckedControlFlowBodyAST::TypeCheckedControlFlowBodyAST(const Type * type,
                                                                     std::unique_ptr<parser::LexerToken> keyword,
                                                                     std::unique_ptr<TypeCheckedBlockAST> block):
            TypeCheckedAST(type), _keyword(std::move(keyword)), _kind(Kind::block) {
            new (&_block) std::unique_ptr<TypeCheckedBlockAST>(std::move(block));
        }

        TypeCheckedControlFlowBodyAST
            ::TypeCheckedControlFlowBodyAST(const Type * type, std::unique_ptr<parser::LexerToken> keyword,
                                            std::unique_ptr<TypeCheckedExpressionAST> expression):
            TypeCheckedAST(type), _keyword(std::move(keyword)), _kind(Kind::expression) {
            new (&_expression) std::unique_ptr<TypeCheckedExpressionAST>(std::move(expression));
        }

        TypeCheckedControlFlowBodyAST::~TypeCheckedControlFlowBodyAST() {
            switch (_kind) {
                case Kind::block: _block.~unique_ptr<TypeCheckedBlockAST>(); break;
                case Kind::expression: _expression.~unique_ptr<TypeCheckedExpressionAST>(); break;
            }
        }

        std::unique_ptr<TypeCheckedControlFlowBodyAST>
        TypeCheckedControlFlowBodyAST::createByTypeChecking(std::unique_ptr<ast::ControlFlowBodyAST> ast,
                                                            const TypeHint & hint,
                                                            diag::DiagnosticEngine & diagnostics) {
            switch (ast->_kind) {
                case ast::ControlFlowBodyAST::Kind::block: {
                    auto block = TypeCheckedBlockAST::createByTypeChecking(std::move(ast->_block), hint, diagnostics);
                    auto type = block->getType();

                    return std::unique_ptr<TypeCheckedControlFlowBodyAST>(
                        new TypeCheckedControlFlowBodyAST(type, std::move(ast->_keyword), std::move(block)));
                }
                case ast::ControlFlowBodyAST::Kind::expression: {
                    auto expression = TypeCheckedExpressionAST::createByTypeChecking(std::move(ast->_expression), hint,
                                                                                     diagnostics);
                    auto type = expression->getType();

                    return std::unique_ptr<TypeCheckedControlFlowBodyAST>(
                        new TypeCheckedControlFlowBodyAST(type, std::move(ast->_keyword), std::move(expression)));
                }
            }
        }
    }
}