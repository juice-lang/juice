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

#include "juice/AST/StatementAST.h"

namespace juice {
    namespace ast {
        ModuleAST::ModuleAST(): _statements() {}

        void ModuleAST::appendStatement(std::unique_ptr<StatementAST> statement) {
            _statements.push_back(std::move(statement));
        }

        void ModuleAST::diagnoseInto(diag::DiagnosticEngine & diagnostics, unsigned level) const {
            for (const auto & statement: _statements) statement->diagnoseInto(diagnostics, level);
        }

        llvm::Value * ModuleAST::codegen(llvm::LLVMContext & context, llvm::IRBuilder<> & builder) const {
            switch (_statements.size()) {
                case 0: return nullptr;
                case 1: return _statements.front()->codegen(context, builder);
                default: {
                    auto last = _statements.end() - 1;
                    for (auto i = _statements.begin(); i < last; ++i)
                        (*i)->codegen(context, builder);
                    return (*last)->codegen(context, builder);
                }
            }
        }
    }
}
