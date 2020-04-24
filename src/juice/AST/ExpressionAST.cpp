// src/juice/Parser/ExpressionAST.cpp - AST nodes for expression parsing
//
// This source file is part of the juice open source project
//
// Copyright (c) 2019 juice project authors
// Licensed under MIT License
//
// See https://github.com/juice-lang/juice/blob/master/LICENSE for license information
// See https://github.com/juice-lang/juice/blob/master/CONTRIBUTORS.txt for the list of juice project authors


#include "juice/AST/ExpressionAST.h"

#include <functional>
#include <map>
#include <utility>

#include "juice/AST/Codegen.h"
#include "juice/AST/CodegenException.h"
#include "juice/Basic/RawStreamHelpers.h"
#include "juice/Basic/SourceLocation.h"
#include "llvm/ADT/APFloat.h"
#include "llvm/ADT/StringRef.h"
#include "llvm/IR/Constants.h"
#include "llvm/IR/Instructions.h"
#include "llvm/IR/Value.h"
#include "llvm/Support/Casting.h"

namespace juice {
    namespace ast {
        static constexpr basic::Color colors[] {
                basic::Color::cyan,
                basic::Color::blue,
                basic::Color::magenta,
                basic::Color::red,
                basic::Color::yellow,
                basic::Color::green
        };

        ExpressionAST::ExpressionAST(Kind kind, std::unique_ptr<juice::parser::LexerToken> token):
            _kind(kind), _token(std::move(token)) {}

        BinaryOperatorExpressionAST::BinaryOperatorExpressionAST(std::unique_ptr<parser::LexerToken> token,
                                                                 std::unique_ptr<ExpressionAST> left,
                                                                 std::unique_ptr<ExpressionAST> right):
            ExpressionAST(Kind::binaryOperator, std::move(token)), _left(std::move(left)), _right(std::move(right)) {}

        void BinaryOperatorExpressionAST::diagnoseInto(diag::DiagnosticEngine & diagnostics, unsigned int level) const {
            basic::SourceLocation location(_token->string.begin());

            diagnostics.diagnose(location, diag::DiagnosticID::binary_operator_expression_ast_0, colors[level % 6],
                                 level, _token.get());
            _left->diagnoseInto(diagnostics, level + 1);
            diagnostics.diagnose(location, diag::DiagnosticID::binary_operator_expression_ast_1, colors[level % 6],
                                 level);
            _right->diagnoseInto(diagnostics, level + 1);
            diagnostics.diagnose(location, diag::DiagnosticID::binary_operator_expression_ast_2, colors[level % 6],
                                 level);
        }

        llvm::Expected<llvm::Value *> BinaryOperatorExpressionAST::codegen(Codegen & state) const {
            using namespace std::placeholders;
            using TokenType = parser::LexerToken::Type;
            using Builder = llvm::IRBuilder<>;

            Builder & builder = state.getBuilder();

            std::map<TokenType, std::function<llvm::Value *(llvm::Value *, llvm::Value *)>> assignmentOperators {
                {TokenType::operatorEqual, nullptr},
                {TokenType::operatorPlusEqual, std::bind(&Builder::CreateFAdd, builder, _1, _2, "addtmp", nullptr)},
                {TokenType::operatorMinusEqual, std::bind(&Builder::CreateFSub, builder, _1, _2, "subtmp", nullptr)},
                {TokenType::operatorAsteriskEqual, std::bind(&Builder::CreateFMul, builder, _1, _2, "multmp", nullptr)},
                {TokenType::operatorSlashEqual, std::bind(&Builder::CreateFDiv, builder, _1, _2, "divtmp", nullptr)}
            };

            auto function = assignmentOperators.find(_token->type);

            if (function != assignmentOperators.end()) {
                auto variable = llvm::dyn_cast_or_null<VariableExpressionAST>(_left.get());

                if (variable == nullptr) {
                    basic::SourceLocation location(_token->string.begin());
                    return llvm::make_error<CodegenError>(diag::DiagnosticID::expression_not_assignable, location);
                }

                llvm::StringRef name = variable->name();

                auto right = _right->codegen(state);
                if (auto error = right.takeError()) return std::move(error);

                auto rightValue = *right;

                if (rightValue == nullptr) return nullptr;

                llvm::AllocaInst * alloca = state.getNamedValue(name);

                if (function->second != nullptr) {
                    llvm::Value * variableValue = builder.CreateLoad(alloca, name);

                    rightValue = function->second(variableValue, rightValue);
                }

                builder.CreateStore(rightValue, alloca);

                return rightValue;
            }

            auto left = _left->codegen(state);
            if (auto error = left.takeError()) return std::move(error);

            auto right = _right->codegen(state);
            if (auto error = right.takeError()) return std::move(error);

            if (*left == nullptr || *right == nullptr) return nullptr;

            switch (_token->type) {
                case parser::LexerToken::Type::operatorPlus:
                    return builder.CreateFAdd(*left, *right, "addtmp");
                case parser::LexerToken::Type::operatorMinus:
                    return builder.CreateFSub(*left, *right, "subtmp");
                case parser::LexerToken::Type::operatorAsterisk:
                    return builder.CreateFMul(*left, *right, "multmp");
                case parser::LexerToken::Type::operatorSlash:
                    return builder.CreateFDiv(*left, *right, "divtmp");
                default:
                    return nullptr;
            }
        }

        NumberExpressionAST::NumberExpressionAST(std::unique_ptr<parser::LexerToken> token, double value):
                ExpressionAST(Kind::number, std::move(token)), _value(value) {}

        void NumberExpressionAST::diagnoseInto(diag::DiagnosticEngine & diagnostics, unsigned int level) const {
            basic::SourceLocation location(_token->string.begin());

            diagnostics.diagnose(location, diag::DiagnosticID::number_expression_ast, colors[level % 6], level,
                                 _token.get(), _value);
        }

        llvm::Expected<llvm::Value *> NumberExpressionAST::codegen(Codegen & state) const {
            return llvm::ConstantFP::get(state.getContext(), llvm::APFloat(_value));
        }

        VariableExpressionAST::VariableExpressionAST(std::unique_ptr<parser::LexerToken> token):
                ExpressionAST(Kind::variable, std::move(token)) {}

        void VariableExpressionAST::diagnoseInto(diag::DiagnosticEngine & diagnostics, unsigned int level) const {
            basic::SourceLocation location(_token->string.begin());
            diagnostics.diagnose(location, diag::DiagnosticID::variable_expression_ast, colors[level % 6],
                                 _token.get());
        }

        llvm::Expected<llvm::Value *> VariableExpressionAST::codegen(Codegen & state) const {
            if (state.namedValueExists(name())) {
                llvm::AllocaInst * alloca = state.getNamedValue(name());

                return state.getBuilder().CreateLoad(alloca, name());
            } else {
                basic::SourceLocation location(name().begin());
                return llvm::make_error<CodegenErrorWithString>(diag::DiagnosticID::unresolved_identifer,
                                                                location, name());
            }
        }

        GroupingExpressionAST::GroupingExpressionAST(std::unique_ptr<parser::LexerToken> token,
                                                     std::unique_ptr<ExpressionAST> expression):
                ExpressionAST(Kind::grouping, std::move(token)), _expression(std::move(expression)) {}

        void GroupingExpressionAST::diagnoseInto(diag::DiagnosticEngine & diagnostics, unsigned int level) const {
            _expression->diagnoseInto(diagnostics, level);
        }

        llvm::Expected<llvm::Value *> GroupingExpressionAST::codegen(Codegen & state) const {
            return _expression->codegen(state);
        }
    }
}
