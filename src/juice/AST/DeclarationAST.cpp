// src/juice/AST/DeclarationAST.cpp - AST nodes for declaration parsing
//
// This source file is part of the juice open source project
//
// Copyright (c) 2019 - 2020 juice project authors
// Licensed under MIT License
//
// See https://github.com/juice-lang/juice/blob/master/LICENSE for license information
// See https://github.com/juice-lang/juice/blob/master/CONTRIBUTORS.txt for the list of juice project authors

#include "juice/AST/DeclarationAST.h"

#include <utility>

#include "juice/AST/Codegen.h"
#include "juice/AST/CodegenError.h"
#include "llvm/ADT/APFloat.h"
#include "llvm/IR/Instructions.h"

namespace juice {
    namespace ast {
        VariableDeclarationAST::VariableDeclarationAST(std::unique_ptr<parser::LexerToken> name,
                                                       std::unique_ptr<ExpressionAST> initialization):
                DeclarationAST(Kind::variableDeclaration), _name(std::move(name)),
                _initialization(std::move(initialization)) {}

        void VariableDeclarationAST::diagnoseInto(diag::DiagnosticEngine & diagnostics, unsigned int level) const {
            basic::SourceLocation location(getLocation());

            diagnostics.diagnose(location, diag::DiagnosticID::variable_declaration_ast, getColor(level), level,
                                 _name.get());
            _initialization->diagnoseInto(diagnostics, level + 1);
            diagnostics.diagnose(location, diag::DiagnosticID::ast_end, getColor(level), level);
        }

        llvm::Expected<llvm::Value *> VariableDeclarationAST::codegen(Codegen & state) const {
            auto value = _initialization->codegen(state);
            if (auto error = value.takeError()) return std::move(error);

            llvm::AllocaInst * alloca = state.getBuilder().CreateAlloca(llvm::Type::getDoubleTy(state.getContext()),
                                                                         nullptr, _name->string);
            state.getBuilder().CreateStore(*value, alloca);

            if (state.newNamedValue(_name->string, alloca)) {
                return nullptr;
            }

            return llvm::make_error<CodegenErrorWithString>(diag::DiagnosticID::invalid_redeclaration, getLocation(),
                                                            _name->string);
        }
    }
}
