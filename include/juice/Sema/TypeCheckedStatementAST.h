// include/juice/Sema/TypeCheckedStatementAST.h - type checked statement AST nodes
//
// This source file is part of the juice open source project
//
// Copyright (c) 2019 - 2020 juice project authors
// Licensed under MIT License
//
// See https://github.com/juice-lang/juice/blob/master/LICENSE for license information
// See https://github.com/juice-lang/juice/blob/master/CONTRIBUTORS.txt for the list of juice project authors


#ifndef JUICE_SEMA_TYPECHECKEDSTATEMENTAST_H
#define JUICE_SEMA_TYPECHECKEDSTATEMENTAST_H

#include <memory>

#include "TypeCheckedAST.h"
#include "TypeCheckedExpressionAST.h"
#include "TypeChecker.h"
#include "juice/AST/StatementAST.h"

namespace juice {
    namespace sema {
        class TypeCheckedStatementAST: public TypeCheckedAST {
            friend class irgen::IRGen;

        protected:
            explicit TypeCheckedStatementAST(Kind kind, Type type): TypeCheckedAST(kind, type) {}

        public:
            TypeCheckedStatementAST() = delete;

            ~TypeCheckedStatementAST() override = default;

            static std::unique_ptr<TypeCheckedStatementAST>
            createByTypeChecking(std::unique_ptr<ast::StatementAST> ast, const TypeHint & hint,
                                 TypeChecker::State & state, diag::DiagnosticEngine & diagnostics);


            static bool classof(const TypeCheckedAST * type) {
                return type->getKind() >= Kind::statement
                    && type->getKind() <= Kind::statement_last;
            }
        };

        class TypeCheckedBlockStatementAST: public TypeCheckedStatementAST {
            std::unique_ptr<TypeCheckedBlockAST> _block;

            TypeCheckedBlockStatementAST(Type type, std::unique_ptr<TypeCheckedBlockAST> block);

            friend class irgen::IRGen;

        public:
            TypeCheckedBlockStatementAST() = delete;

            ~TypeCheckedBlockStatementAST() override = default;

            basic::SourceLocation getLocation() const override {
                return _block->getLocation();
            }

            void diagnoseInto(diag::DiagnosticEngine & diagnostics, unsigned int level) const override;

            static std::unique_ptr<TypeCheckedBlockStatementAST>
            createByTypeChecking(std::unique_ptr<ast::BlockStatementAST> ast, const TypeHint & hint,
                                 TypeChecker::State & state, diag::DiagnosticEngine & diagnostics);


            static bool classof(const TypeCheckedAST * type) {
                return type->getKind() == Kind::blockStatement;
            }
        };

        class TypeCheckedExpressionStatementAST: public TypeCheckedStatementAST {
            std::unique_ptr<TypeCheckedExpressionAST> _expression;

            TypeCheckedExpressionStatementAST(Type type, std::unique_ptr<TypeCheckedExpressionAST> expression);

            friend class irgen::IRGen;

        public:
            TypeCheckedExpressionStatementAST() = delete;

            ~TypeCheckedExpressionStatementAST() override = default;

            basic::SourceLocation getLocation() const override {
                return _expression->getLocation();
            }

            void diagnoseInto(diag::DiagnosticEngine & diagnostics, unsigned int level) const override;

            static std::unique_ptr<TypeCheckedExpressionStatementAST>
            createByTypeChecking(std::unique_ptr<ast::ExpressionStatementAST> ast, const TypeHint & hint,
                                 TypeChecker::State & state, diag::DiagnosticEngine & diagnostics);


            static bool classof(const TypeCheckedAST * type) {
                return type->getKind() == Kind::expressionStatement;
            }
        };

        class TypeCheckedIfStatementAST: public TypeCheckedStatementAST {
            std::unique_ptr<TypeCheckedIfExpressionAST> _ifExpression;

            TypeCheckedIfStatementAST(Type type, std::unique_ptr<TypeCheckedIfExpressionAST> ifExpression);

            friend class irgen::IRGen;

        public:
            TypeCheckedIfStatementAST() = delete;

            ~TypeCheckedIfStatementAST() override = default;

            basic::SourceLocation getLocation() const override {
                return _ifExpression->getLocation();
            }

            void diagnoseInto(diag::DiagnosticEngine & diagnostics, unsigned int level) const override;

            static std::unique_ptr<TypeCheckedIfStatementAST>
            createByTypeChecking(std::unique_ptr<ast::IfStatementAST> ast, const TypeHint & hint,
                                 TypeChecker::State & state, diag::DiagnosticEngine & diagnostics);


            static bool classof(const TypeCheckedAST * type) {
                return type->getKind() == Kind::ifStatement;
            }
        };

        class TypeCheckedWhileStatementAST: public TypeCheckedStatementAST {
            std::unique_ptr<TypeCheckedExpressionAST> _condition;
            std::unique_ptr<TypeCheckedControlFlowBodyAST> _body;

            TypeCheckedWhileStatementAST(Type type, std::unique_ptr<TypeCheckedExpressionAST> condition,
                                         std::unique_ptr<TypeCheckedControlFlowBodyAST> body);

            friend class irgen::IRGen;

        public:
            TypeCheckedWhileStatementAST() = delete;

            ~TypeCheckedWhileStatementAST() override = default;

            basic::SourceLocation getLocation() const override {
                return _body->getLocation();
            }

            void diagnoseInto(diag::DiagnosticEngine & diagnostics, unsigned int level) const override;

            static std::unique_ptr<TypeCheckedWhileStatementAST>
            createByTypeChecking(std::unique_ptr<ast::WhileStatementAST> ast, const TypeHint & hint,
                                 TypeChecker::State & state, diag::DiagnosticEngine & diagnostics);


            static bool classof(const TypeCheckedAST * type) {
                return type->getKind() == Kind::whileStatement;
            }
        };
    }
}

#endif //JUICE_SEMA_TYPECHECKEDSTATEMENTAST_H
