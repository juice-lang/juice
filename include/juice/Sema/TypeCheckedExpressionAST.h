// include/juice/Sema/TypeCheckedExpressionAST.h - type checked expression AST nodes
//
// This source file is part of the juice open source project
//
// Copyright (c) 2019 - 2020 juice project authors
// Licensed under MIT License
//
// See https://github.com/juice-lang/juice/blob/master/LICENSE for license information
// See https://github.com/juice-lang/juice/blob/master/CONTRIBUTORS.txt for the list of juice project authors


#ifndef JUICE_SEMA_TYPECHECKEDEXPRESSIONAST_H
#define JUICE_SEMA_TYPECHECKEDEXPRESSIONAST_H

#include <memory>
#include <utility>
#include <vector>

#include "TypeCheckedAST.h"
#include "TypeChecker.h"
#include "VariableDeclaration.h"
#include "juice/AST/ExpressionAST.h"
#include "juice/Basic/SourceLocation.h"
#include "juice/Parser/LexerToken.h"
#include "llvm/ADT/StringRef.h"

namespace juice {
    namespace sema {
        class TypeCheckedExpressionAST: public TypeCheckedAST {
            friend class irgen::IRGen;

        protected:
            std::unique_ptr<parser::LexerToken> _token;

            TypeCheckedExpressionAST(Kind kind, Type type, std::unique_ptr<parser::LexerToken> token);

            static void checkLValue(const TypeHint & hint, basic::SourceLocation location,
                                    diag::DiagnosticEngine & diagnostics, llvm::StringRef name);
            static void checkType(Type type, const TypeHint & hint, basic::SourceLocation location,
                                  diag::DiagnosticEngine & diagnostics);

        public:
            TypeCheckedExpressionAST() = delete;

            ~TypeCheckedExpressionAST() override = default;

            basic::SourceLocation getLocation() const override {
                return basic::SourceLocation(_token->string.begin());
            }

            static std::unique_ptr<TypeCheckedExpressionAST>
            createByTypeChecking(std::unique_ptr<ast::ExpressionAST> ast, const TypeHint & hint,
                                 TypeChecker::State & state, diag::DiagnosticEngine & diagnostics);


            static bool classof(const TypeCheckedAST * type) {
                return type->getKind() >= Kind::expression
                    && type->getKind() <= Kind::expression_last;
            }
        };

        class TypeCheckedBinaryOperatorExpressionAST: public TypeCheckedExpressionAST {
            std::unique_ptr<TypeCheckedExpressionAST> _left, _right;

            TypeCheckedBinaryOperatorExpressionAST(Type type, std::unique_ptr<parser::LexerToken> token,
                                                   std::unique_ptr<TypeCheckedExpressionAST> left,
                                                   std::unique_ptr<TypeCheckedExpressionAST> right);

            friend class irgen::IRGen;

        public:
            TypeCheckedBinaryOperatorExpressionAST() = delete;

            void diagnoseInto(diag::DiagnosticEngine & diagnostics, unsigned int level) const override;

            static std::unique_ptr<TypeCheckedBinaryOperatorExpressionAST>
            createByTypeChecking(std::unique_ptr<ast::BinaryOperatorExpressionAST> ast, const TypeHint & hint,
                                 TypeChecker::State & state, diag::DiagnosticEngine & diagnostics);


            static bool classof(const TypeCheckedAST * ast) {
                return ast->getKind() == Kind::binaryOperatorExpression;
            }
        };

        class TypeCheckedIntegerLiteralExpressionAST: public TypeCheckedExpressionAST {
            int64_t _value;

            TypeCheckedIntegerLiteralExpressionAST(Type type, std::unique_ptr<parser::LexerToken> token, int64_t value);

            friend class irgen::IRGen;

        public:
            TypeCheckedIntegerLiteralExpressionAST() = delete;

            void diagnoseInto(diag::DiagnosticEngine & diagnostics, unsigned int level) const override;

            static std::unique_ptr<TypeCheckedIntegerLiteralExpressionAST>
            createByTypeChecking(std::unique_ptr<ast::IntegerLiteralExpressionAST> ast, const TypeHint & hint,
                                 TypeChecker::State & state, diag::DiagnosticEngine & diagnostics);


            static bool classof(const TypeCheckedAST * ast) {
                return ast->getKind() == Kind::integerLiteralExpression;
            }
        };

        class TypeCheckedFloatingPointLiteralExpressionAST: public TypeCheckedExpressionAST {
            double _value;

            TypeCheckedFloatingPointLiteralExpressionAST(Type type, std::unique_ptr<parser::LexerToken> token,
                                                         double value);

            friend class irgen::IRGen;

        public:
            TypeCheckedFloatingPointLiteralExpressionAST() = delete;

            void diagnoseInto(diag::DiagnosticEngine & diagnostics, unsigned int level) const override;

            static std::unique_ptr<TypeCheckedFloatingPointLiteralExpressionAST>
            createByTypeChecking(std::unique_ptr<ast::FloatingPointLiteralExpressionAST> ast, const TypeHint & hint,
                                 TypeChecker::State & state, diag::DiagnosticEngine & diagnostics);


            static bool classof(const TypeCheckedAST * ast) {
                return ast->getKind() == Kind::floatingPointLiteralExpression;
            }
        };

        class TypeCheckedBooleanLiteralExpressionAST: public TypeCheckedExpressionAST {
            bool _value;

            TypeCheckedBooleanLiteralExpressionAST(Type type, std::unique_ptr<parser::LexerToken> token, bool value);

            friend class irgen::IRGen;

        public:
            TypeCheckedBooleanLiteralExpressionAST() = delete;

            void diagnoseInto(diag::DiagnosticEngine & diagnostics, unsigned int level) const override;

            static std::unique_ptr<TypeCheckedBooleanLiteralExpressionAST>
            createByTypeChecking(std::unique_ptr<ast::BooleanLiteralExpressionAST> ast, const TypeHint & hint,
                                 TypeChecker::State & state, diag::DiagnosticEngine & diagnostics);


            static bool classof(const TypeCheckedAST * ast) {
                return ast->getKind() == Kind::booleanLiteralExpression;
            }
        };

        class TypeCheckedVariableExpressionAST: public TypeCheckedExpressionAST {
            size_t _index;
            bool _isMutable;

            TypeCheckedVariableExpressionAST(std::unique_ptr<parser::LexerToken> token,
                                             VariableDeclaration declaration);

            friend class irgen::IRGen;

        public:
            TypeCheckedVariableExpressionAST() = delete;

            llvm::StringRef name() const { return _token->string; }

            void diagnoseInto(diag::DiagnosticEngine & diagnostics, unsigned int level) const override;

            static std::unique_ptr<TypeCheckedVariableExpressionAST>
            createByTypeChecking(std::unique_ptr<ast::VariableExpressionAST> ast, const TypeHint & hint,
                                 TypeChecker::State & state, diag::DiagnosticEngine & diagnostics);


            static bool classof(const TypeCheckedAST * ast) {
                return ast->getKind() == Kind::variableExpression;
            }
        };

        class TypeCheckedGroupingExpressionAST: public TypeCheckedExpressionAST {
            std::unique_ptr<TypeCheckedExpressionAST> _expression;

            TypeCheckedGroupingExpressionAST(Type type, std::unique_ptr<parser::LexerToken> token,
                                             std::unique_ptr<TypeCheckedExpressionAST> expression);

            friend class irgen::IRGen;

        public:
            TypeCheckedGroupingExpressionAST() = delete;

            void diagnoseInto(diag::DiagnosticEngine & diagnostics, unsigned int level) const override;

            static std::unique_ptr<TypeCheckedGroupingExpressionAST>
            createByTypeChecking(std::unique_ptr<ast::GroupingExpressionAST> ast, const TypeHint & hint,
                                 TypeChecker::State & state, diag::DiagnosticEngine & diagnostics);


            static bool classof(const TypeCheckedAST * ast) {
                return ast->getKind() == Kind::groupingExpression;
            }
        };

        class TypeCheckedIfExpressionAST: public TypeCheckedExpressionAST {
        public:
            typedef std::vector<std::pair<std::unique_ptr<TypeCheckedExpressionAST>,
                                          std::unique_ptr<TypeCheckedControlFlowBodyAST>>> ElifVector;

        private:
            std::unique_ptr<TypeCheckedExpressionAST> _ifCondition;
            std::unique_ptr<TypeCheckedControlFlowBodyAST> _ifBody;
            ElifVector _elifConditionsAndBodies;
            std::unique_ptr<TypeCheckedControlFlowBodyAST> _elseBody;
            bool _isStatement;

            TypeCheckedIfExpressionAST(Type type, std::unique_ptr<TypeCheckedExpressionAST> ifCondition,
                                       std::unique_ptr<TypeCheckedControlFlowBodyAST> ifBody,
                                       ElifVector && elifConditionsAndBodies,
                                       std::unique_ptr<TypeCheckedControlFlowBodyAST> elseBody, bool isStatement);

            friend class TypeCheckedIfStatementAST;
            friend class irgen::IRGen;

        public:
            TypeCheckedIfExpressionAST() = delete;

            basic::SourceLocation getLocation() const override {
                return _ifBody->getLocation();
            }

            void diagnoseInto(diag::DiagnosticEngine & diagnostics, unsigned int level) const override;

            static std::unique_ptr<TypeCheckedIfExpressionAST>
            createByTypeChecking(std::unique_ptr<ast::IfExpressionAST> ast, const TypeHint & hint,
                                 TypeChecker::State & state, diag::DiagnosticEngine & diagnostics);


            static bool classof(const TypeCheckedAST * ast) {
                return ast->getKind() == Kind::ifExpression;
            }
        };
    }
}

#endif //JUICE_SEMA_TYPECHECKEDEXPRESSIONAST_H
