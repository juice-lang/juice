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

#include <utility>

namespace juice {
    namespace parser {
        BinaryOperatorExpressionAST::BinaryOperatorExpressionAST(std::unique_ptr<LexerToken> operatorToken,
                                                                 std::unique_ptr<ExpressionAST> left,
                                                                 std::unique_ptr<ExpressionAST> right):
                _operatorToken(std::move(operatorToken)), _left(std::move(left)), _right(std::move(right)) {}
    }
}
