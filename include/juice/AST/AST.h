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

            virtual void diagnoseInto(diag::DiagnosticEngine & diagnostics, unsigned int level) const = 0;

            virtual llvm::Value * codegen(Codegen & state) const = 0;
        };

        class StatementAST;

        class ContainerAST: public AST {
        protected:
            std::vector<std::unique_ptr<StatementAST>> _statements;

        public:
            ContainerAST() = default;

            void appendStatement(std::unique_ptr<StatementAST> statement);
        };

        class ModuleAST: public ContainerAST {
        public:
            ModuleAST() = default;

            ~ModuleAST() override = default;

            void diagnoseInto(diag::DiagnosticEngine & diagnostics, unsigned int level) const override;

            llvm::Value * codegen(Codegen & state) const override;
        };

        class BlockAST: public ContainerAST {
            std::unique_ptr<parser::LexerToken> _start;

        public:
            BlockAST() = delete;

            BlockAST(std::unique_ptr<parser::LexerToken> start);

            ~BlockAST() override = default;

            void diagnoseInto(diag::DiagnosticEngine & diagnostics, unsigned int level) const override;

            llvm::Value * codegen(Codegen & state) const override;
        };
    }
}

#endif //JUICE_AST_H
