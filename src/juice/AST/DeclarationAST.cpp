// src/juice/AST/DeclarationAST.cpp - AST nodes for declaration parsing
//
// This source file is part of the juice open source project
//
// Copyright (c) 2019 juice project authors
// Licensed under MIT License
//
// See https://github.com/juice-lang/juice/blob/master/LICENSE for license information
// See https://github.com/juice-lang/juice/blob/master/CONTRIBUTORS.txt for the list of juice project authors

#include "juice/AST/DeclarationAST.h"

#include <utility>

#include "llvm/ADT/APFloat.h"
#include "llvm/IR/Instructions.h"

namespace juice {
    namespace ast {
        VariableDeclarationAST::VariableDeclarationAST(std::unique_ptr<parser::LexerToken> name,
                                                       std::unique_ptr<ExpressionAST> initialization) :
            _name(std::move(name)), _initialization(std::move(initialization)) {}

        void VariableDeclarationAST::diagnoseInto(diag::DiagnosticEngine &diagnostics, unsigned int level) const {
            basic::SourceLocation location(_name->string.begin());
            std::string indentation;
            for (unsigned i = 0; i < level; ++i) {
                indentation += "    ";
            }
            llvm::StringRef indentationRef(indentation);

            diagnostics.diagnose(location, diag::DiagnosticID::variable_declaration_ast_0, indentationRef, _name.get());
            _initialization->diagnoseInto(diagnostics, level + 1);
            diagnostics.diagnose(location, diag::DiagnosticID::variable_declaration_ast_1, indentationRef);
        }

        llvm::Value * VariableDeclarationAST::codegen(llvm::LLVMContext &context, llvm::IRBuilder<> &builder) const {
            llvm::Value * value = _initialization->codegen(context, builder);

            llvm::AllocaInst * alloca = builder.CreateAlloca(llvm::Type::getDoubleTy(context), nullptr, _name->string);
            builder.CreateStore(value, alloca);

            return alloca;
        }
    }
}
