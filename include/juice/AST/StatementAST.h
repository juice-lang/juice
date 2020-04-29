// include/juice/AST/StatementAST.h - AST nodes for statement parsing
//
// This source file is part of the juice open source project
//
// Copyright (c) 2019 - 2020 juice project authors
// Licensed under MIT License
//
// See https://github.com/juice-lang/juice/blob/master/LICENSE for license information
// See https://github.com/juice-lang/juice/blob/master/CONTRIBUTORS.txt for the list of juice project authors


#ifndef JUICE_AST_STATEMENTAST_H
#define JUICE_AST_STATEMENTAST_H

#include <memory>

#include "AST.h"
#include "ExpressionAST.h"

namespace juice {
    namespace sema {
        class TypeCheckedBlockStatementAST;
        class TypeCheckedExpressionStatementAST;
        class TypeCheckedIfStatementAST;
        class TypeCheckedWhileStatementAST;
    }

    namespace ast {
        class StatementAST: public AST {
        public:
            enum class Kind {
                block,
                expression,
                _if,
                _while,
                declaration,
                variableDeclaration,
                declaration_last
            };

        private:
            const Kind _kind;

        protected:
            explicit StatementAST(Kind kind): _kind(kind) {}

        public:
            StatementAST() = delete;

            ~StatementAST() override = default;

            Kind getKind() const { return _kind; }
        };

        class BlockStatementAST: public StatementAST {
            std::unique_ptr<BlockAST> _block;

            friend class sema::TypeCheckedBlockStatementAST;

        public:
            BlockStatementAST() = delete;

            explicit BlockStatementAST(std::unique_ptr<BlockAST> block);

            ~BlockStatementAST() override = default;

            basic::SourceLocation getLocation() const override {
                return _block->getLocation();
            }

            void diagnoseInto(diag::DiagnosticEngine & diagnostics, unsigned int level) const override;

            llvm::Expected<llvm::Value *> codegen(Codegen & state) const override;


            static bool classof(const StatementAST * ast) {
                return ast->getKind() == Kind::block;
            }
        };

        class ExpressionStatementAST: public StatementAST {
            std::unique_ptr<ExpressionAST> _expression;

            friend class sema::TypeCheckedExpressionStatementAST;

        public:
            ExpressionStatementAST() = delete;

            explicit ExpressionStatementAST(std::unique_ptr<ExpressionAST> expression);

            ~ExpressionStatementAST() override = default;

            basic::SourceLocation getLocation() const override {
                return _expression->getLocation();
            }

            void diagnoseInto(diag::DiagnosticEngine & diagnostics, unsigned int level) const override;

            llvm::Expected<llvm::Value *> codegen(Codegen & state) const override;


            static bool classof(const StatementAST * ast) {
                return ast->getKind() == Kind::expression;
            }
        };

        class IfStatementAST: public StatementAST {
            std::unique_ptr<IfExpressionAST> _ifExpression;

            friend class sema::TypeCheckedIfStatementAST;

        public:
            IfStatementAST() = delete;

            explicit IfStatementAST(std::unique_ptr<IfExpressionAST> ifExpression);

            ~IfStatementAST() override = default;

            basic::SourceLocation getLocation() const override {
                return _ifExpression->getLocation();
            }

            void diagnoseInto(diag::DiagnosticEngine & diagnostics, unsigned int level) const override;

            llvm::Expected<llvm::Value *> codegen(Codegen & state) const override;


            static bool classof(const StatementAST * ast) {
                return ast->getKind() == Kind::_if;
            }
        };

        class WhileStatementAST: public StatementAST {
            std::unique_ptr<ExpressionAST> _condition;
            std::unique_ptr<ControlFlowBodyAST> _body;

            friend class sema::TypeCheckedWhileStatementAST;

        public:
            WhileStatementAST() = delete;

            WhileStatementAST(std::unique_ptr<ExpressionAST> condition, std::unique_ptr<ControlFlowBodyAST> body);

            ~WhileStatementAST() override = default;

            basic::SourceLocation getLocation() const override {
                return _body->getLocation();
            }

            void diagnoseInto(diag::DiagnosticEngine & diagnostics, unsigned int level) const override;

            llvm::Expected<llvm::Value *> codegen(Codegen & state) const override;


            static bool classof(const StatementAST * ast) {
                return ast->getKind() == Kind::_while;
            }
        };
    }
}

#endif //JUICE_AST_STATEMENTAST_H
