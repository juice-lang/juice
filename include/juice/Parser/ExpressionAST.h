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

#include "LexerToken.h"

namespace juice {
    namespace parser {
        class ExpressionAST {
        protected:
            std::unique_ptr<LexerToken> _token;

        public:
            ExpressionAST(std::unique_ptr<LexerToken> token);

            virtual ~ExpressionAST() = default;
        };

        class BinaryOperatorExpressionAST: public ExpressionAST {
            std::unique_ptr<ExpressionAST> _left, _right;

        public:
            BinaryOperatorExpressionAST(std::unique_ptr<LexerToken> token, std::unique_ptr<ExpressionAST> left,
                                        std::unique_ptr<ExpressionAST> right);
        };

        class NumberExpressionAST: public ExpressionAST {
            double _value;

        public:
            explicit NumberExpressionAST(std::unique_ptr<LexerToken> token, double value);
        };

        class GroupingExpressionAST: public ExpressionAST {
            std::unique_ptr<ExpressionAST> _expression;

        public:
            explicit GroupingExpressionAST(std::unique_ptr<LexerToken> token,
                                           std::unique_ptr<ExpressionAST> expression);
        };
    }
}

#endif //JUICE_EXPRESSIONAST_H
