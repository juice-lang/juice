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
#include "juice/AST/ExpressionAST.h"
#include "juice/Parser/LexerToken.h"

namespace juice {
    namespace sema {
        class TypeCheckedExpressionAST: public TypeCheckedAST {
        public:
            enum class Kind {
                binaryOperator,
                number,
                variable,
                grouping,
                _if
            };

        private:
            const Kind _kind;

        protected:
            std::unique_ptr<parser::LexerToken> _token;

            TypeCheckedExpressionAST(const Type * type, Kind kind, std::unique_ptr<parser::LexerToken> token);

        public:
            TypeCheckedExpressionAST() = delete;

            ~TypeCheckedExpressionAST() override = default;

            basic::SourceLocation getLocation() const override {
                return basic::SourceLocation(_token->string.begin());
            }

            Kind getKind() const { return _kind; }

            static std::unique_ptr<TypeCheckedExpressionAST>
            createByTypeChecking(std::unique_ptr<ast::ExpressionAST> ast, const TypeHint & hint,
                                 TypeChecker::State & state, diag::DiagnosticEngine & diagnostics);
        };

        class TypeCheckedBinaryOperatorExpressionAST: public TypeCheckedExpressionAST {
            std::unique_ptr<TypeCheckedExpressionAST> _left, _right;

            TypeCheckedBinaryOperatorExpressionAST(const Type * type, std::unique_ptr<parser::LexerToken> token,
                                                   std::unique_ptr<TypeCheckedExpressionAST> left,
                                                   std::unique_ptr<TypeCheckedExpressionAST> right);

        public:
            TypeCheckedBinaryOperatorExpressionAST() = delete;

            void diagnoseInto(diag::DiagnosticEngine & diagnostics, unsigned int level) const override;

            static std::unique_ptr<TypeCheckedBinaryOperatorExpressionAST>
            createByTypeChecking(std::unique_ptr<ast::BinaryOperatorExpressionAST> ast, const TypeHint & hint,
                                 TypeChecker::State & state, diag::DiagnosticEngine & diagnostics);


            static bool classof(const TypeCheckedExpressionAST * ast) {
                return ast->getKind() == Kind::binaryOperator;
            }
        };

        class TypeCheckedNumberExpressionAST: public TypeCheckedExpressionAST {
            double _value;

            TypeCheckedNumberExpressionAST(const Type * type, std::unique_ptr<parser::LexerToken> token, double value);

        public:
            TypeCheckedNumberExpressionAST() = delete;

            void diagnoseInto(diag::DiagnosticEngine & diagnostics, unsigned int level) const override;

            static std::unique_ptr<TypeCheckedNumberExpressionAST>
            createByTypeChecking(std::unique_ptr<ast::NumberExpressionAST> ast, const TypeHint & hint,
                                 TypeChecker::State & state, diag::DiagnosticEngine & diagnostics);


            static bool classof(const TypeCheckedExpressionAST * ast) {
                return ast->getKind() == Kind::number;
            }
        };

        class TypeCheckedVariableExpressionAST: public TypeCheckedExpressionAST {
            size_t _index;

            TypeCheckedVariableExpressionAST(const Type * type, std::unique_ptr<parser::LexerToken> token,
                                             size_t index);

        public:
            TypeCheckedVariableExpressionAST() = delete;

            llvm::StringRef name() const { return _token->string; }

            void diagnoseInto(diag::DiagnosticEngine & diagnostics, unsigned int level) const override;

            static std::unique_ptr<TypeCheckedVariableExpressionAST>
            createByTypeChecking(std::unique_ptr<ast::VariableExpressionAST> ast, const TypeHint & hint,
                                 TypeChecker::State & state, diag::DiagnosticEngine & diagnostics);


            static bool classof(const TypeCheckedExpressionAST * ast) {
                return ast->getKind() == Kind::variable;
            }
        };

        class TypeCheckedGroupingExpressionAST: public TypeCheckedExpressionAST {
            std::unique_ptr<TypeCheckedExpressionAST> _expression;

            TypeCheckedGroupingExpressionAST(const Type * type, std::unique_ptr<parser::LexerToken> token,
                                             std::unique_ptr<TypeCheckedExpressionAST> expression);

        public:
            TypeCheckedGroupingExpressionAST() = delete;

            void diagnoseInto(diag::DiagnosticEngine & diagnostics, unsigned int level) const override;

            static std::unique_ptr<TypeCheckedGroupingExpressionAST>
            createByTypeChecking(std::unique_ptr<ast::GroupingExpressionAST> ast, const TypeHint & hint,
                                 TypeChecker::State & state, diag::DiagnosticEngine & diagnostics);


            static bool classof(const TypeCheckedExpressionAST * ast) {
                return ast->getKind() == Kind::grouping;
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

            TypeCheckedIfExpressionAST(const Type * type, std::unique_ptr<TypeCheckedExpressionAST> ifCondition,
                                       std::unique_ptr<TypeCheckedControlFlowBodyAST> ifBody,
                                       ElifVector && elifConditionsAndBodies,
                                       std::unique_ptr<TypeCheckedControlFlowBodyAST> elseBody, bool isStatement);

            friend class TypeCheckedIfStatementAST;

        public:
            TypeCheckedIfExpressionAST() = delete;

            basic::SourceLocation getLocation() const override {
                return _ifBody->getLocation();
            }

            void diagnoseInto(diag::DiagnosticEngine & diagnostics, unsigned int level) const override;

            static std::unique_ptr<TypeCheckedIfExpressionAST>
            createByTypeChecking(std::unique_ptr<ast::IfExpressionAST> ast, const TypeHint & hint,
                                 TypeChecker::State & state, diag::DiagnosticEngine & diagnostics);


            static bool classof(const TypeCheckedExpressionAST * ast) {
                return ast->getKind() == Kind::_if;
            }
        };
    }
}

#endif //JUICE_SEMA_TYPECHECKEDEXPRESSIONAST_H
