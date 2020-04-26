// include/juice/Parser/ExpressionAST.h - AST nodes for expression parsing
//
// This source file is part of the juice open source project
//
// Copyright (c) 2019 juice project authors
// Licensed under MIT License
//
// See https://github.com/juice-lang/juice/blob/master/LICENSE for license information
// See https://github.com/juice-lang/juice/blob/master/CONTRIBUTORS.txt for the list of juice project authors


#ifndef JUICE_EXPRESSIONAST_H
#define JUICE_EXPRESSIONAST_H

#include <memory>
#include <utility>
#include <vector>

#include "AST.h"
#include "juice/Parser/LexerToken.h"
#include "juice/Diagnostics/Diagnostics.h"
#include "llvm/IR/IRBuilder.h"
#include "llvm/IR/LLVMContext.h"
#include "llvm/IR/Value.h"

namespace juice {
    namespace ast {
        class ExpressionAST: public AST {
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

        public:
            ExpressionAST() = delete;

            explicit ExpressionAST(Kind kind, std::unique_ptr<parser::LexerToken> token);

            ~ExpressionAST() override = default;

            basic::SourceLocation getLocation() const override {
                return basic::SourceLocation(_token->string.begin());
            }

            Kind getKind() const { return _kind; }
        };

        class BinaryOperatorExpressionAST: public ExpressionAST {
            std::unique_ptr<ExpressionAST> _left, _right;

        public:
            BinaryOperatorExpressionAST() = delete;

            BinaryOperatorExpressionAST(std::unique_ptr<parser::LexerToken> token, std::unique_ptr<ExpressionAST> left,
                                        std::unique_ptr<ExpressionAST> right);

            void diagnoseInto(diag::DiagnosticEngine & diagnostics, unsigned int level) const override;

            llvm::Expected<llvm::Value *> codegen(Codegen & state) const override;


            static bool classof(const ExpressionAST * ast) {
                return ast->getKind() == Kind::binaryOperator;
            }
        };

        class NumberExpressionAST: public ExpressionAST {
            double _value;

        public:
            NumberExpressionAST() = delete;

            NumberExpressionAST(std::unique_ptr<parser::LexerToken> token, double value);

            void diagnoseInto(diag::DiagnosticEngine & diagnostics, unsigned int level) const override;

            llvm::Expected<llvm::Value *> codegen(Codegen & state) const override;


            static bool classof(const ExpressionAST * ast) {
                return ast->getKind() == Kind::number;
            }
        };

        class VariableExpressionAST: public ExpressionAST {
        public:
            VariableExpressionAST() = delete;

            explicit VariableExpressionAST(std::unique_ptr<parser::LexerToken> token);

            llvm::StringRef name() const { return _token->string; }

            void diagnoseInto(diag::DiagnosticEngine &diagnostics, unsigned int level) const override;

            llvm::Expected<llvm::Value *> codegen(Codegen &state) const override;


            static bool classof(const ExpressionAST * ast) {
                return ast->getKind() == Kind::variable;
            }
        };

        class GroupingExpressionAST: public ExpressionAST {
            std::unique_ptr<ExpressionAST> _expression;

        public:
            GroupingExpressionAST() = delete;

            GroupingExpressionAST(std::unique_ptr<parser::LexerToken> token, std::unique_ptr<ExpressionAST> expression);

            void diagnoseInto(diag::DiagnosticEngine & diagnostics, unsigned int level) const override;

            llvm::Expected<llvm::Value *> codegen(Codegen & state) const override;


            static bool classof(const ExpressionAST * ast) {
                return ast->getKind() == Kind::grouping;
            }
        };

        class IfExpressionAST: public ExpressionAST {
            std::unique_ptr<ExpressionAST> _ifCondition;
            std::unique_ptr<IfBodyAST> _ifBody;
            std::vector<std::pair<std::unique_ptr<ExpressionAST>, std::unique_ptr<IfBodyAST>>> _elifConditionsAndBodies;
            std::unique_ptr<IfBodyAST> _elseBody;
            bool _isStatement;

            friend class IfStatementAST;

        public:
            IfExpressionAST() = delete;

            IfExpressionAST(std::unique_ptr<ExpressionAST> ifCondition, std::unique_ptr<IfBodyAST> ifBody,
                            std::vector<std::pair<std::unique_ptr<ExpressionAST>,
                                                  std::unique_ptr<IfBodyAST>>> && elifConditionsAndBodies,
                            std::unique_ptr<IfBodyAST> elseBody, bool isStatement);

            basic::SourceLocation getLocation() const override {
                return _ifBody->getLocation();
            }

            void diagnoseInto(diag::DiagnosticEngine & diagnostics, unsigned int level) const override;

            llvm::Expected<llvm::Value *> codegen(Codegen & state) const override;
        };
    }
}

#endif //JUICE_EXPRESSIONAST_H
