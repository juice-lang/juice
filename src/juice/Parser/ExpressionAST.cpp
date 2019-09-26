// src/juice/Parser/ExpressionAST.cpp - AST nodes for expression parsing
//
// This source file is part of the juice open source project
//
// Copyright (c) 2019 juice project authors
// Licensed under MIT License
//
// See https://github.com/juice-lang/juice/blob/master/LICENSE for license information
// See https://github.com/juice-lang/juice/blob/master/CONTRIBUTORS.txt for the list of juice project authors


#include "juice/Parser/ExpressionAST.h"

#include <string>
#include <utility>

#include "juice/Basic/SourceLocation.h"
#include "juice/Basic/StringRef.h"
#include "llvm/ADT/APFloat.h"
#include "llvm/IR/Constants.h"
#include "termcolor/termcolor.hpp"

namespace juice {
    namespace parser {
        static const diag::Color colors[] {
            termcolor::cyan,
            termcolor::blue,
            termcolor::magenta,
            termcolor::red,
            termcolor::yellow,
            termcolor::green
        };

        ExpressionAST::ExpressionAST(std::unique_ptr<juice::parser::LexerToken> token): _token(std::move(token)) {}

        BinaryOperatorExpressionAST::BinaryOperatorExpressionAST(std::unique_ptr<LexerToken> token,
                                                                 std::unique_ptr<ExpressionAST> left,
                                                                 std::unique_ptr<ExpressionAST> right):
                ExpressionAST(std::move(token)), _left(std::move(left)), _right(std::move(right)) {}

        void BinaryOperatorExpressionAST::diagnoseInto(diag::DiagnosticEngine & diagnostics, unsigned level) {
            basic::SourceLocation location(_token->string.begin());
            std::string indentation;
            for (unsigned i = 0; i < level; ++i) {
                indentation += "    ";
            }
            basic::StringRef indentationRef(indentation);
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
        BinaryOperatorExpressionAST::codegen(llvm::LLVMContext & context, llvm::IRBuilder<> & builder) {
            llvm::Value * left = _left->codegen(context, builder);
            llvm::Value * right = _right->codegen(context, builder);

            if (left == nullptr || right == nullptr) return nullptr;

            switch (_token->type) {
                case LexerToken::Type::operatorPlus: return builder.CreateFAdd(left, right, "addtmp");
                case LexerToken::Type::operatorMinus: return builder.CreateFSub(left, right, "subtmp");
                case LexerToken::Type::operatorAsterisk: return builder.CreateFMul(left, right, "multmp");
                case LexerToken::Type::operatorSlash: return builder.CreateFDiv(left, right, "divtmp");
                default: return nullptr;
            }
        }

        NumberExpressionAST::NumberExpressionAST(std::unique_ptr<LexerToken> token, double value):
                ExpressionAST(std::move(token)), _value(value) {}

        void NumberExpressionAST::diagnoseInto(diag::DiagnosticEngine & diagnostics, unsigned level) {
            basic::SourceLocation location(_token->string.begin());
            std::string indentation;
            for (unsigned i = 0; i < level; ++i) {
                indentation += "    ";
            }
            basic::StringRef indentationRef(indentation);
            diagnostics.diagnose(location, diag::DiagnosticID::number_expression_ast, colors[level % 6], indentationRef,
                                 _token.get(), _value);
        }

        llvm::Value * NumberExpressionAST::codegen(llvm::LLVMContext & context, llvm::IRBuilder<> & builder) {
            return llvm::ConstantFP::get(context, llvm::APFloat(_value));
        }

        GroupingExpressionAST::GroupingExpressionAST(std::unique_ptr<LexerToken> token,
                                                     std::unique_ptr<juice::parser::ExpressionAST> expression):
                ExpressionAST(std::move(token)), _expression(std::move(expression)) {}

        void GroupingExpressionAST::diagnoseInto(diag::DiagnosticEngine & diagnostics, unsigned level) {
            _expression->diagnoseInto(diagnostics, level);
        }

        llvm::Value * GroupingExpressionAST::codegen(llvm::LLVMContext & context, llvm::IRBuilder<> & builder) {
            return _expression->codegen(context, builder);
        }
    }
}
