// include/juice/AST/AST.h - basic AST nodes
//
// This source file is part of the juice open source project
//
// Copyright (c) 2019 - 2020 juice project authors
// Licensed under MIT License
//
// See https://github.com/juice-lang/juice/blob/master/LICENSE for license information
// See https://github.com/juice-lang/juice/blob/master/CONTRIBUTORS.txt for the list of juice project authors


#ifndef JUICE_AST_AST_H
#define JUICE_AST_AST_H

#include <memory>
#include <vector>

#include "juice/Basic/RawStreamHelpers.h"
#include "juice/Basic/SourceLocation.h"
#include "juice/Diagnostics/Diagnostics.h"
#include "juice/Parser/LexerToken.h"
#include "llvm/IR/IRBuilder.h"
#include "llvm/IR/LLVMContext.h"
#include "llvm/IR/Value.h"
#include "llvm/Support/Error.h"

namespace juice {
    namespace ast {
        class Codegen;
        class StatementAST;
        class ExpressionAST;


        class AST {
        protected:
            static basic::Color getColor(unsigned int level) {
                return basic::Color::rainbow[level % 6];
            }

        public:
            virtual ~AST() = default;

            virtual basic::SourceLocation getLocation() const = 0;

            virtual void diagnoseInto(diag::DiagnosticEngine & diagnostics, unsigned int level) const = 0;

            virtual llvm::Expected<llvm::Value *> codegen(Codegen & state) const = 0;
        };


        class ContainerAST: public AST {
        protected:
            std::vector<std::unique_ptr<StatementAST>> _statements;

        public:
            ContainerAST() = default;

            basic::SourceLocation getLocation() const override;

            void appendStatement(std::unique_ptr<StatementAST> statement);
        };

        class ModuleAST: public ContainerAST {
        public:
            ModuleAST() = default;

            ~ModuleAST() override = default;

            void diagnoseInto(diag::DiagnosticEngine & diagnostics, unsigned int level) const override;

            llvm::Expected<llvm::Value *> codegen(Codegen & state) const override;
        };

        class BlockAST: public ContainerAST {
            std::unique_ptr<parser::LexerToken> _start;

        public:
            BlockAST() = delete;

            explicit BlockAST(std::unique_ptr<parser::LexerToken> start);

            ~BlockAST() override = default;

            basic::SourceLocation getLocation() const override {
                return basic::SourceLocation(_start->string.begin());
            }

            void diagnoseInto(diag::DiagnosticEngine & diagnostics, unsigned int level) const override;

            llvm::Expected<llvm::Value *> codegen(Codegen & state) const override;
        };


        class ControlFlowBodyAST: public AST {
            enum class Kind {
                block,
                expression
            };

            std::unique_ptr<parser::LexerToken> _keyword;

            Kind _kind;
            union {
                std::unique_ptr<BlockAST> _block;
                std::unique_ptr<ExpressionAST> _expression;
            };

        public:
            ControlFlowBodyAST() = delete;

            ControlFlowBodyAST(std::unique_ptr<parser::LexerToken> keyword, std::unique_ptr<BlockAST> block);
            ControlFlowBodyAST(std::unique_ptr<parser::LexerToken> keyword, std::unique_ptr<ExpressionAST> expression);

            ~ControlFlowBodyAST() override;

            basic::SourceLocation getLocation() const override {
                return basic::SourceLocation(_keyword->string.begin());
            }

            void diagnoseInto(diag::DiagnosticEngine & diagnostics, unsigned int level) const override;

            llvm::Expected<llvm::Value *> codegen(Codegen & state) const override;


            const std::unique_ptr<parser::LexerToken> & getKeyword() const { return _keyword; }
        };
    }
}

#endif //JUICE_AST_AST_H
