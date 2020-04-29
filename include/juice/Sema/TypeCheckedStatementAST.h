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
#include "juice/AST/StatementAST.h"

namespace juice {
    namespace sema {
        class TypeCheckedStatementAST: public TypeCheckedAST {
        protected:
            explicit TypeCheckedStatementAST(const Type * type): TypeCheckedAST(type) {}

        public:
            TypeCheckedStatementAST() = delete;

            ~TypeCheckedStatementAST() override = default;

            static std::unique_ptr<TypeCheckedStatementAST>
            createByTypeChecking(std::unique_ptr<ast::StatementAST> ast, const TypeHint & hint,
                                 diag::DiagnosticEngine & diagnostics);
        };

        class TypeCheckedBlockStatementAST: public TypeCheckedStatementAST {
            std::unique_ptr<TypeCheckedBlockAST> _block;

            TypeCheckedBlockStatementAST(const Type * type, std::unique_ptr<TypeCheckedBlockAST> block);

        public:
            TypeCheckedBlockStatementAST() = delete;

            ~TypeCheckedBlockStatementAST() override = default;

            basic::SourceLocation getLocation() const override {
                return _block->getLocation();
            }

            static std::unique_ptr<TypeCheckedBlockStatementAST>
            createByTypeChecking(std::unique_ptr<ast::BlockStatementAST> ast, const TypeHint & hint,
                                 diag::DiagnosticEngine & diagnostics);
        };

        class TypeCheckedExpressionStatementAST: public TypeCheckedStatementAST {
            std::unique_ptr<TypeCheckedExpressionAST> _expression;

            TypeCheckedExpressionStatementAST(const Type * type, std::unique_ptr<TypeCheckedExpressionAST> expression);

        public:
            TypeCheckedExpressionStatementAST() = delete;

            ~TypeCheckedExpressionStatementAST() override = default;

            basic::SourceLocation getLocation() const override {
                return _expression->getLocation();
            }

            static std::unique_ptr<TypeCheckedExpressionStatementAST>
            createByTypeChecking(std::unique_ptr<ast::ExpressionStatementAST> ast, const TypeHint & hint,
                                 diag::DiagnosticEngine & diagnostics);
        };

        class TypeCheckedIfStatementAST: public TypeCheckedStatementAST {
            std::unique_ptr<TypeCheckedIfExpressionAST> _ifExpression;

            TypeCheckedIfStatementAST(const Type * type, std::unique_ptr<TypeCheckedIfExpressionAST> ifExpression);

        public:
            TypeCheckedIfStatementAST() = delete;

            ~TypeCheckedIfStatementAST() override = default;

            basic::SourceLocation getLocation() const override {
                return _ifExpression->getLocation();
            }

            static std::unique_ptr<TypeCheckedIfStatementAST>
            createByTypeChecking(std::unique_ptr<ast::IfStatementAST> ast, const TypeHint & hint,
                                 diag::DiagnosticEngine & diagnostics);
        };

        class TypeCheckedWhileStatementAST: public TypeCheckedStatementAST {
            std::unique_ptr<TypeCheckedExpressionAST> _condition;
            std::unique_ptr<TypeCheckedControlFlowBodyAST> _body;

            TypeCheckedWhileStatementAST(const Type * type, std::unique_ptr<TypeCheckedExpressionAST> condition,
                                         std::unique_ptr<TypeCheckedControlFlowBodyAST> body);

        public:
            TypeCheckedWhileStatementAST() = delete;

            ~TypeCheckedWhileStatementAST() override = default;

            basic::SourceLocation getLocation() const override {
                return _body->getLocation();
            }

            static std::unique_ptr<TypeCheckedWhileStatementAST>
            createByTypeChecking(std::unique_ptr<ast::WhileStatementAST> ast, const TypeHint & hint,
                                 diag::DiagnosticEngine & diagnostics);
        };
    }
}

#endif //JUICE_SEMA_TYPECHECKEDSTATEMENTAST_H