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

        std::unique_ptr<ast::ExpressionAST> Parser::parsePrimaryExpression() {
            if (match(LexerToken::Type::integerLiteral) || match(LexerToken::Type::decimalLiteral)) {
                auto token = std::move(_previousToken);
                double number = std::stod(token->string.str());
                return std::make_unique<ast::NumberExpressionAST>(std::move(token), number);
            } else if (match(LexerToken::Type::identifier)) {
                auto token = std::move(_previousToken);
                return std::make_unique<ast::VariableExpressionAST>(std::move(token));
            }
            return parseGroupedExpression();
        }

        std::unique_ptr<ast::ExpressionAST> Parser::parseMultiplicationPrecedenceExpression() {
            auto node = parsePrimaryExpression();

            while (match(LexerToken::Type::operatorAsterisk) || match(LexerToken::Type::operatorSlash)) {
                auto token = std::move(_previousToken);
                auto right = parsePrimaryExpression();
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

        std::unique_ptr<ast::ExpressionStatementAST> Parser::parseExpressionStatement() {
            auto expression = parseAdditionPrecedenceExpression();

            if (!match(LexerToken::Type::delimiterNewline) && !isAtEnd()) {
                consume(LexerToken::Type::delimiterSemicolon, diag::DiagnosticID::expression_statement_expected_newline_or_semicolon);
            }

            return std::make_unique<ast::ExpressionStatementAST>(std::move(expression));
        }

        std::unique_ptr<ast::VariableDeclarationAST> Parser::parseVariableDeclaration() {
            consume(LexerToken::Type::identifier, diag::DiagnosticID::expected_variable_name);

            auto name = std::move(_previousToken);

            consume(LexerToken::Type::operatorEqual, diag::DiagnosticID::expected_variable_initialization);

            auto initialization = parseAdditionPrecedenceExpression();

            if (!match(LexerToken::Type::delimiterNewline) && !isAtEnd()) {
                consume(LexerToken::Type::delimiterSemicolon, diag::DiagnosticID::variable_declaration_expected_newline_or_semicolon);
            }

            return std::make_unique<ast::VariableDeclarationAST>(std::move(name), std::move(initialization));
        }

        std::unique_ptr<ast::StatementAST> Parser::parseStatement() {
            if (match(LexerToken::Type::keywordVar)) {
                return parseVariableDeclaration();
            }

            return parseExpressionStatement();
        }

        std::unique_ptr<ast::ModuleAST> Parser::parseModule() {
            auto module = std::make_unique<ast::ModuleAST>();

            advance();
            while (!isAtEnd()) {
                module->appendStatement(parseStatement());
            }

            return module;
        }


        Parser::Parser(std::shared_ptr<diag::DiagnosticEngine> diagnostics):
                _diagnostics(std::move(diagnostics)), _previousToken(nullptr), _currentToken(nullptr) {
            _lexer = std::make_unique<Lexer>(_diagnostics->getBuffer());
        }

        std::unique_ptr<ast::ModuleAST> Parser::parseProgram() {
            try {
                return parseModule();
            } catch (const Error & error) {
                basic::SourceLocation location(_currentToken->string.begin());
                _diagnostics->diagnose(location, error.id);
            } catch (const LexerError & error) {
                _currentToken->diagnoseInto(*_diagnostics);
            }

            return nullptr;
        }
    }
}
