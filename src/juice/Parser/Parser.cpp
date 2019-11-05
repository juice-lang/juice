// src/juice/Parser/Parser.cpp - Parser class, parses source file into an AST
//
// This source file is part of the juice open source project
//
// Copyright (c) 2019 juice project authors
// Licensed under MIT License
//
// See https://github.com/juice-lang/juice/blob/master/LICENSE for license information
// See https://github.com/juice-lang/juice/blob/master/CONTRIBUTORS.txt for the list of juice project authors


#include "juice/Parser/Parser.h"

#include <string>
#include <utility>

#include "juice/Basic/SourceLocation.h"

namespace juice {
    namespace parser {
        bool Parser::isAtEnd() {
            return _currentToken != nullptr && _currentToken->type == LexerToken::Type::eof;
        }

        bool Parser::check(LexerToken::Type type) {
            if (isAtEnd()) return false;
            return _currentToken->type == type;
        }

        void Parser::advance() {
            _previousToken = std::move(_currentToken);
            if (!isAtEnd()) _currentToken = _lexer->nextToken();
            if (check(LexerToken::Type::error)) throw LexerError();
        }

        bool Parser::match(LexerToken::Type type) {
            if (check(type)) {
                advance();
                return true;
            }
            return false;
        }

        void Parser::consume(LexerToken::Type type, diag::DiagnosticID errorID) {
            if (check(type)) advance();
            else throw Error(errorID);
        }

        std::unique_ptr<ast::ExpressionAST> Parser::parseGroupedExpression() {
            if (match(LexerToken::Type::delimiterLeftParen)) {
                auto token = std::move(_previousToken);
                auto expression = parseAdditionPrecedenceExpression();
                consume(LexerToken::Type::delimiterRightParen, diag::DiagnosticID::expected_right_paren);
                return std::make_unique<ast::GroupingExpressionAST>(std::move(token), std::move(expression));
            }
            throw Error(diag::DiagnosticID::expected_expression);
        }

        std::unique_ptr<ast::ExpressionAST> Parser::parseNumberExpression() {
            if (match(LexerToken::Type::integerLiteral) || match(LexerToken::Type::decimalLiteral)) {
                auto token = std::move(_previousToken);
                double number = std::stod(token->string.str());
                return std::make_unique<ast::NumberExpressionAST>(std::move(token), number);
            }
            return parseGroupedExpression();
        }

        std::unique_ptr<ast::ExpressionAST> Parser::parseMultiplicationPrecedenceExpression() {
            auto node = parseNumberExpression();

            while (match(LexerToken::Type::operatorAsterisk) || match(LexerToken::Type::operatorSlash)) {
                auto token = std::move(_previousToken);
                auto right = parseNumberExpression();
                node = std::make_unique<ast::BinaryOperatorExpressionAST>(std::move(token), std::move(node),
                                                                     std::move(right));
            }

            return node;
        }

        std::unique_ptr<ast::ExpressionAST> Parser::parseAdditionPrecedenceExpression() {
            auto node = parseMultiplicationPrecedenceExpression();

            while (match(LexerToken::Type::operatorPlus) || match(LexerToken::Type::operatorMinus)) {
                auto token = std::move(_previousToken);
                auto right = parseMultiplicationPrecedenceExpression();
                node = std::make_unique<ast::BinaryOperatorExpressionAST>(std::move(token), std::move(node),
                                                                     std::move(right));
            }

            return node;
        }

        Parser::Parser(std::shared_ptr<diag::DiagnosticEngine> diagnostics):
                _diagnostics(std::move(diagnostics)), _previousToken(nullptr), _currentToken(nullptr) {
            _lexer = std::make_unique<Lexer>(_diagnostics->getBuffer());
        }

        std::unique_ptr<ast::ExpressionAST> Parser::parseProgram() {
            try {
                advance();
                auto expression = parseAdditionPrecedenceExpression();

                consume(LexerToken::Type::delimiterNewline, diag::DiagnosticID::expected_newline);

                if (!isAtEnd()) throw Error(diag::DiagnosticID::expected_end_of_file);

                return expression;
            } catch (Error & error) {
                basic::SourceLocation location(_currentToken->string.begin());
                _diagnostics->diagnose(location, error.id);
            } catch (LexerError & error) {
                _currentToken->diagnoseInto(*_diagnostics);
            }

            return nullptr;
        }
    }
}
