// src/juice/AST/AST.cpp - basic AST nodes
//
// This source file is part of the juice open source project
//
// Copyright (c) 2019 - 2020 juice project authors
// Licensed under MIT License
//
// See https://github.com/juice-lang/juice/blob/master/LICENSE for license information
// See https://github.com/juice-lang/juice/blob/master/CONTRIBUTORS.txt for the list of juice project authors

#include "juice/AST/AST.h"

#include <utility>

#include "juice/AST/Codegen.h"
#include "juice/AST/StatementAST.h"

namespace juice {
    namespace ast {
        basic::SourceLocation ContainerAST::getLocation() const {
            if (!_statements.empty()) return _statements.front()->getLocation();
            return {};
        }

        void ContainerAST::appendStatement(std::unique_ptr<StatementAST> statement) {
            _statements.push_back(std::move(statement));
        }

        void ModuleAST::diagnoseInto(diag::DiagnosticEngine & diagnostics, unsigned int level) const {
            for (const auto & statement: _statements) { statement->diagnoseInto(diagnostics, level); }
        }

        llvm::Expected<llvm::Value *> ModuleAST::codegen(Codegen & state) const {
            switch (_statements.size()) {
                case 0:
                    return nullptr;
                case 1:
                    return _statements.front()->codegen(state);
                default: {
                    auto last = _statements.end() - 1;
                    for (auto i = _statements.begin(); i < last; ++i) {
                        if (auto error = (*i)->codegen(state).takeError()) return std::move(error);
                    }
                    return (*last)->codegen(state);
                }
            }
        }

        BlockAST::BlockAST(std::unique_ptr<parser::LexerToken> start): ContainerAST(), _start(std::move(start)) {}

        void BlockAST::diagnoseInto(diag::DiagnosticEngine & diagnostics, unsigned int level) const {
            basic::SourceLocation location(getLocation());

            if (_statements.empty()) {
                diagnostics.diagnose(location, diag::DiagnosticID::block_ast_empty, getColor(level));
            } else {
                diagnostics.diagnose(location, diag::DiagnosticID::block_ast_0, getColor(level), level);

                for (const auto & statement: _statements) {
                    diagnostics.diagnose(location, diag::DiagnosticID::block_ast_1, level + 1);

                    statement->diagnoseInto(diagnostics, level + 1);
                }

                diagnostics.diagnose(location, diag::DiagnosticID::block_ast_2, getColor(level), level);
            }
        }

        llvm::Expected<llvm::Value *> BlockAST::codegen(Codegen & state) const {
            state.newScope();

            llvm::Value * returnValue;

            switch (_statements.size()) {
                case 0:
                    returnValue = nullptr;
                    break;
                case 1: {
                    auto value = _statements.front()->codegen(state);
                    if (auto error = value.takeError()) return std::move(error);

                    returnValue = *value;
                    break;
                }
                default: {
                    auto last = _statements.end() - 1;
                    for (auto i = _statements.begin(); i < last; ++i) {
                        if (auto error = (*i)->codegen(state).takeError()) return std::move(error);
                    }

                    auto value = (*last)->codegen(state);
                    if (auto error = value.takeError()) return std::move(error);

                    returnValue = *value;
                    break;
                }
            }

            state.endScope();

            return returnValue;
        }

        ControlFlowBodyAST::ControlFlowBodyAST(std::unique_ptr<parser::LexerToken> keyword, std::unique_ptr<BlockAST> block):
            _keyword(std::move(keyword)), _kind(Kind::block) {
            new (&_block) std::unique_ptr<BlockAST>(std::move(block));
        }

        ControlFlowBodyAST::ControlFlowBodyAST(std::unique_ptr<parser::LexerToken> keyword,
                                               std::unique_ptr<ExpressionAST> expression):
            _keyword(std::move(keyword)), _kind(Kind::expression) {
            new (&_expression) std::unique_ptr<ExpressionAST>(std::move(expression));
        }

        ControlFlowBodyAST::~ControlFlowBodyAST() {
            switch (_kind) {
                case Kind::block: _block.~unique_ptr<BlockAST>(); break;
                case Kind::expression: _expression.~unique_ptr<ExpressionAST>(); break;
            }
        }

        void ControlFlowBodyAST::diagnoseInto(diag::DiagnosticEngine & diagnostics, unsigned int level) const {
            basic::SourceLocation location(getLocation());

            switch (_kind) {
                case Kind::block:
                    diagnostics.diagnose(location, diag::DiagnosticID::if_body_ast_block, getColor(level), level,
                                         _keyword.get());
                    _block->diagnoseInto(diagnostics, level + 1);
                    break;
                case Kind::expression:
                    diagnostics.diagnose(location, diag::DiagnosticID::if_body_ast_expression, getColor(level), level,
                                         _keyword.get());
                    _expression->diagnoseInto(diagnostics, level + 1);
                    break;
            }

            diagnostics.diagnose(location, diag::DiagnosticID::ast_end, getColor(level), level);
        }

        llvm::Expected<llvm::Value *> ControlFlowBodyAST::codegen(Codegen & state) const {
            switch (_kind) {
                case Kind::block: return _block->codegen(state);
                case Kind::expression: return _expression->codegen(state);
            }
        }
    }
}
