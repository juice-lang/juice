// include/juice/AST/AST.h - basic AST nodes
//
// This source file is part of the juice open source project
//
// Copyright (c) 2019 juice project authors
// Licensed under MIT License
//
// See https://github.com/juice-lang/juice/blob/master/LICENSE for license information
// See https://github.com/juice-lang/juice/blob/master/CONTRIBUTORS.txt for the list of juice project authors


#ifndef JUICE_AST_H
#define JUICE_AST_H

#include <memory>
#include <vector>

#include "juice/Diagnostics/Diagnostics.h"
#include "llvm/IR/IRBuilder.h"
#include "llvm/IR/LLVMContext.h"
#include "llvm/IR/Value.h"

namespace juice {
    namespace ast {
        class Codegen;

        class AST {
        public:
            virtual ~AST() = default;

            virtual void diagnoseInto(diag::DiagnosticEngine & diagnostics, unsigned level) const = 0;

            virtual llvm::Value * codegen(Codegen & state) const = 0;

            void diagnoseInto(diag::DiagnosticEngine & diagnostics) const { diagnoseInto(diagnostics, 0); }
        };

        class StatementAST;

        class ModuleAST: public AST {
            std::vector<std::unique_ptr<StatementAST>> _statements;

        public:
            ModuleAST();

            ~ModuleAST() override = default;

            void appendStatement(std::unique_ptr<StatementAST> statement);

            void diagnoseInto(diag::DiagnosticEngine & diagnostics, unsigned level) const override;

            llvm::Value * codegen(Codegen & state) const override;
        };
    }
}

#endif //JUICE_AST_H
