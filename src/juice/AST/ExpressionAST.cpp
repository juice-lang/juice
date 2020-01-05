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

#include <string>
#include <utility>

#include "juice/AST/Codegen.h"
#include "juice/AST/CodegenException.h"
#include "juice/Basic/RawStreamHelpers.h"
#include "juice/Basic/SourceLocation.h"
#include "llvm/ADT/APFloat.h"
#include "llvm/ADT/StringRef.h"
#include "llvm/IR/Constants.h"
#include "llvm/IR/Instructions.h"

namespace juice {
    namespace ast {
        static const basic::Color colors[] {
                basic::Color::cyan,
                basic::Color::blue,
                basic::Color::magenta,
                basic::Color::red,
                basic::Color::yellow,
                basic::Color::green
        };

        ExpressionAST::ExpressionAST(std::unique_ptr<juice::parser::LexerToken> token): _token(std::move(token)) {}

        BinaryOperatorExpressionAST::BinaryOperatorExpressionAST(std::unique_ptr<parser::LexerToken> token,
                                                                 std::unique_ptr<ExpressionAST> left,
                                                                 std::unique_ptr<ExpressionAST> right):
                ExpressionAST(std::move(token)), _left(std::move(left)), _right(std::move(right)) {}

        void BinaryOperatorExpressionAST::diagnoseInto(diag::DiagnosticEngine & diagnostics, unsigned level) const {
            basic::SourceLocation location(_token->string.begin());
            std::string indentation;
            for (unsigned i = 0; i < level; ++i) {
                indentation += "    ";
            }
            llvm::StringRef indentationRef(indentation);
            diagnostics.diagnose(location, diag::DiagnosticID::binary_operator_expression_ast_0, colors[level % 6],
                                 indentationRef, _token.get());
            _left->diagnoseInto(diagnostics, level + 1);
            diagnostics.diagnose(location, diag::DiagnosticID::binary_operator_expression_ast_1, colors[level % 6],
                                 indentationRef);
            _right->diagnoseInto(diagnostics, level + 1);
            diagnostics.diagnose(location, diag::DiagnosticID::binary_operator_expression_ast_2, colors[level % 6],
                                 indentationRef);
        }

        llvm::Value *
        BinaryOperatorExpressionAST::codegen(Codegen & state) const {
            llvm::Value * left = _left->codegen(state);
            llvm::Value * right = _right->codegen(state);

            if (left == nullptr || right == nullptr) return nullptr;

            llvm::IRBuilder<> & builder = state.getBuilder();

            switch (_token->type) {
                case parser::LexerToken::Type::operatorPlus:
                    return builder.CreateFAdd(left, right, "addtmp");
                case parser::LexerToken::Type::operatorMinus:
                    return builder.CreateFSub(left, right, "subtmp");
                case parser::LexerToken::Type::operatorAsterisk:
                    return builder.CreateFMul(left, right, "multmp");
                case parser::LexerToken::Type::operatorSlash:
                    return builder.CreateFDiv(left, right, "divtmp");
                default:
                    return nullptr;
            }
        }

        NumberExpressionAST::NumberExpressionAST(std::unique_ptr<parser::LexerToken> token, double value):
                ExpressionAST(std::move(token)), _value(value) {}

        void NumberExpressionAST::diagnoseInto(diag::DiagnosticEngine & diagnostics, unsigned level) const {
            basic::SourceLocation location(_token->string.begin());
            std::string indentation;
            for (unsigned i = 0; i < level; ++i) {
                indentation += "    ";
            }
            llvm::StringRef indentationRef(indentation);
            diagnostics.diagnose(location, diag::DiagnosticID::number_expression_ast, colors[level % 6], indentationRef,
                                 _token.get(), _value);
        }

        llvm::Value * NumberExpressionAST::codegen(Codegen & state) const {
            return llvm::ConstantFP::get(state.getContext(), llvm::APFloat(_value));
        }

        VariableExpressionAST::VariableExpressionAST(std::unique_ptr<parser::LexerToken> token):
                ExpressionAST(std::move(token)) {}

        void VariableExpressionAST::diagnoseInto(diag::DiagnosticEngine & diagnostics, unsigned int level) const {
            basic::SourceLocation location(_token->string.begin());
            diagnostics.diagnose(location, diag::DiagnosticID::variable_expression_ast, colors[level % 6],
                                 _token.get());
        }

        llvm::Value * VariableExpressionAST::codegen(Codegen & state) const {
            llvm::StringRef name = _token->string;
            if (state.namedValueExists(name)) {
                llvm::AllocaInst * alloca = state.getNamedValue(name);

                return state.getBuilder().CreateLoad(alloca, name);
            } else {
                basic::SourceLocation location(name.begin());
                throw VariableException(diag::DiagnosticID::unresolved_identifer, location, name);
            }
        }

        GroupingExpressionAST::GroupingExpressionAST(std::unique_ptr<parser::LexerToken> token,
                                                     std::unique_ptr<ExpressionAST> expression):
                ExpressionAST(std::move(token)), _expression(std::move(expression)) {}

        void GroupingExpressionAST::diagnoseInto(diag::DiagnosticEngine & diagnostics, unsigned level) const {
            _expression->diagnoseInto(diagnostics, level);
        }

        llvm::Value * GroupingExpressionAST::codegen(Codegen & state) const {
            return _expression->codegen(state);
        }
    }
}
