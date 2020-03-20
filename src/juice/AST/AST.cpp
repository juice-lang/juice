// src/juice/AST/AST.cpp - basic AST nodes
//
// This source file is part of the juice open source project
//
// Copyright (c) 2019 juice project authors
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
        void ContainerAST::appendStatement(std::unique_ptr<StatementAST> statement) {
            _statements.push_back(std::move(statement));
        }

        void ModuleAST::diagnoseInto(diag::DiagnosticEngine & diagnostics, unsigned int level) const {
            for (const auto & statement: _statements) { statement->diagnoseInto(diagnostics, level); }
        }

        llvm::Value * ModuleAST::codegen(Codegen & state) const {
            switch (_statements.size()) {
                case 0:
                    return nullptr;
                case 1:
                    return _statements.front()->codegen(state);
                default: {
                    auto last = _statements.end() - 1;
                    for (auto i = _statements.begin(); i < last; ++i) {
                        (*i)->codegen(state);
                    }
                    return (*last)->codegen(state);
                }
            }
        }

        BlockAST::BlockAST(std::unique_ptr<parser::LexerToken> start): ContainerAST(), _start(std::move(start)) {}

        void BlockAST::diagnoseInto(diag::DiagnosticEngine & diagnostics, unsigned int level) const {
            basic::SourceLocation location(_start->string.begin());

            if (_statements.empty()) {
                diagnostics.diagnose(location, diag::DiagnosticID::block_ast_empty);
            } else {
                diagnostics.diagnose(location, diag::DiagnosticID::block_ast_0, level);

                for (const auto & statement: _statements) {
                    diagnostics.diagnose(location, diag::DiagnosticID::block_ast_1, level + 1);

                    statement->diagnoseInto(diagnostics, level + 1);
                }

                diagnostics.diagnose(location, diag::DiagnosticID::block_ast_2, level);
            }
        }

        llvm::Value * BlockAST::codegen(Codegen & state) const {
            state.newScope();

            llvm::Value * returnValue;

            switch (_statements.size()) {
                case 0:
                    returnValue = nullptr;
                    break;
                case 1:
                    returnValue = _statements.front()->codegen(state);
                    break;
                default: {
                    auto last = _statements.end() - 1;
                    for (auto i = _statements.begin(); i < last; ++i) {
                        (*i)->codegen(state);
                    }
                    returnValue = (*last)->codegen(state);
                    break;
                }
            }

            state.endScope();

            return returnValue;
        }
    }
}
